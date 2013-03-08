#include "calc.hpp"
#include "chess.hpp"
#include "config.hpp"
#include "endgame.hpp"
#include "eval.hpp"
#include "eval_values.hpp"
#include "fen.hpp"
#include "moves.hpp"
#include "pawn_structure_hash_table.hpp"
#include "see.hpp"
#include "selftest.hpp"
#include "util/mutex.hpp"
#include "util/time.hpp"
#include "util/string.hpp"
#include "util.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <vector>


namespace {
void checking( std::string const& what, bool endline = false )
{
	std::cout << "Checking " << what << "... ";
	if( endline ) {
		std::cout << std::endl;
	}
	else {
		std::cout.flush();
	}
}

void pass()
{
	std::cout << "pass" << std::endl;
}
}

struct perft_ctx {
	perft_ctx()
		: move_ptr(moves)
	{
	}

	move_info moves[200 * (MAX_DEPTH) ];
	move_info* move_ptr;

	killer_moves killers;
};


template<bool split_movegen>
void perft( perft_ctx& ctx, int depth, position const& p, uint64_t& n )
{
	move_info* moves = ctx.move_ptr;

	check_map check( p );
	if( split_movegen ) {
		calculate_moves<movegen_type::capture>( p, ctx.move_ptr, check );
		calculate_moves<movegen_type::noncapture>( p, ctx.move_ptr, check );
	}
	else {
		calculate_moves<movegen_type::all>( p, ctx.move_ptr, check );
	}

	if( !--depth ) {
		n += ctx.move_ptr - moves;
		ctx.move_ptr = moves;
		return;
	}

	for( move_info* it = moves; it != ctx.move_ptr; ++it ) {
		position new_pos(p);
		apply_move( new_pos, it->m );
		perft<split_movegen>( ctx, depth, new_pos, n );
	}
	ctx.move_ptr = moves;
}

template<bool split_movegen>
void perft( position const& p, std::size_t max_depth, uint64_t const* perft_results, std::size_t size )
{
	perft_ctx ctx;
	for( unsigned int i = 0; i < std::min(max_depth, size); ++i ) {
		ctx.move_ptr = ctx.moves;

		std::cerr << "Calculating number of possible moves in " << (i + 1) << " plies:" << std::endl;

		uint64_t ret = 0;

		int max_depth = i + 1;

		timestamp start;
		perft<split_movegen>( ctx, max_depth, p, ret );
		timestamp stop;

		duration elapsed = stop - start;

		std::cerr << "Moves:     "     << ret << std::endl;
		std::cerr << "Took:      "     << elapsed.milliseconds() << " ms" << std::endl;
		if( ret ) {
			// Will overflow after ~3 months
			int64_t picoseconds = elapsed.picoseconds();
			picoseconds /= ret;

			std::stringstream ss;
			ss << "Time/move: " << picoseconds / 1000 << "." << std::setw(1) << std::setfill('0') << (picoseconds / 100) % 10 << " ns" << std::endl;

			if( !elapsed.empty() ) {
				ss << "Moves/s:   " << elapsed.get_items_per_second(ret) << std::endl;
			}

			std::cerr << ss.str();
		}

		if( perft_results[i] != 0 && ret != perft_results[i] ) {
			std::cerr << "FAIL! Expected " << perft_results[i] << " moves." << std::endl;
			abort();
		}
		else {
			std::cerr << "PASS" << std::endl;
		}
		std::cerr << std::endl;
	}
}
template void perft<true>( std::size_t max_depth );
template void perft<false>( std::size_t max_depth );

template<bool split_movegen>
void perft( std::size_t max_depth )
{
	position p;
	uint64_t const perft_results[] = {
		20ull,
		400ull,
		8902ull,
		197281ull,
		4865609ull,
		119060324ull,
		3195901860ull,
		84998978956ull
	};
	perft<split_movegen>( p, max_depth, perft_results, sizeof(perft_results) / sizeof(uint64_t) );
}

namespace {

position test_parse_fen( std::string const& fen )
{
	position p;
	std::string error;
	if( !parse_fen( fen, p, &error ) ) {
		std::cerr << "Could not parse fen: " << error << std::endl;
		std::cerr << "Fen: " << fen << std::endl;
		abort();
	}

	return p;
}

move test_parse_move( position const& p, std::string const& ms )
{
	move m;
	std::string error;
	if( !parse_move( p, ms, m, error ) ) {
		std::cerr << error << ": " << ms << std::endl;
		abort();
	}

	return m;
}

static void test_move_generation( std::string const& fen, std::string const& ref_moves )
{
	position p = test_parse_fen( fen );

	std::vector<std::string> ms;
	for( auto m : calculate_moves<movegen_type::all>( p ) ) {
		ms.push_back( move_to_string( p, m ) );
	}
	std::sort( ms.begin(), ms.end() );

	std::string s;
	for( std::size_t i = 0; i < ms.size(); ++i ) {
		if( i ) {
			s += " ";
		}
		std::size_t pos;
		while( (pos = ms[i].find( ' ' ) ) != std::string::npos ) {
			ms[i].erase( pos, 1 );
		}
		s += ms[i];
	}

	if( ref_moves != s ) {
		std::cerr << "Move mismatch!" << std::endl;
		std::cerr << "Fen: " << fen << std::endl;
		std::cerr << "Reference: " << ref_moves << std::endl;
		std::cerr << "Actual:    " << s << std::endl;
		abort();
	}
}


static void test_move_generation()
{
	checking("move generation");

	test_move_generation( "4R3/p1pp1p1p/b1n1rn2/1p2p1pP/1B1QPBr1/qPPb2PN/Pk1PNP2/R3K3 w Q g6",
						"c3-c4 f2-f3 h5-h6 h5xg6 Bb4-a5 Bb4-c5 Bb4-d6 Bb4-e7 Bb4-f8 Bb4xa3 Bf4-e3 Bf4xe5 Bf4xg5 Ke1-d1 Ke1-f1 Ne2-c1 Ne2-g1 Nh3-g1 Nh3xg5 Qd4-b6 Qd4-c4 Qd4-c5 Qd4-d5 Qd4-d6 Qd4-e3 Qd4xa7 Qd4xd3 Qd4xd7 Qd4xe5 Ra1-b1 Ra1-c1 Ra1-d1 Re8-a8 Re8-b8 Re8-c8 Re8-d8 Re8-e7 Re8-f8 Re8-g8 Re8-h8 Re8xe6" );
	test_move_generation( "r3k2r/8/2p5/1Q6/1q6/2P5/8/R3K2R w KqQk -",
						"O-O O-O-O c3xb4 Ke1-d1 Ke1-d2 Ke1-e2 Ke1-f1 Ke1-f2 Qb5-a4 Qb5-a5 Qb5-a6 Qb5-b6 Qb5-b7 Qb5-b8 Qb5-c4 Qb5-c5 Qb5-d3 Qb5-d5 Qb5-e2 Qb5-e5 Qb5-f1 Qb5-f5 Qb5-g5 Qb5-h5 Qb5xb4 Qb5xc6 Ra1-a2 Ra1-a3 Ra1-a4 Ra1-a5 Ra1-a6 Ra1-a7 Ra1-b1 Ra1-c1 Ra1-d1 Ra1xa8 Rh1-f1 Rh1-g1 Rh1-h2 Rh1-h3 Rh1-h4 Rh1-h5 Rh1-h6 Rh1-h7 Rh1xh8" );
	test_move_generation( "3k4/8/8/q2pP2K/8/8/8/8 w - d6",
						"e5-e6 Kh5-g4 Kh5-g5 Kh5-g6 Kh5-h4 Kh5-h6");
	test_move_generation( "3k4/8/8/q2pPP1K/8/8/8/8 w - d6",
						"e5-e6 e5xd6 f5-f6 Kh5-g4 Kh5-g5 Kh5-g6 Kh5-h4 Kh5-h6");
	test_move_generation( "3k4/8/8/K2pP2q/8/8/8/8 w - d6",
						"e5-e6 Ka5-a4 Ka5-a6 Ka5-b4 Ka5-b5 Ka5-b6");
	test_move_generation( "3k4/8/8/K2pPP1q/8/8/8/8 w - d6",
						"e5-e6 e5xd6 f5-f6 Ka5-a4 Ka5-a6 Ka5-b4 Ka5-b5 Ka5-b6");
	test_move_generation( "r1bqkb1r/ppp1ppPp/2n5/8/8/5N2/PpPP1PPP/R1BQKB1R w KQkq -",
						"a2-a3 a2-a4 c2-c3 c2-c4 d2-d3 d2-d4 g2-g3 g2-g4 g7-g8=B g7-g8=N g7-g8=Q g7-g8=R g7xf8=B g7xf8=N g7xf8=Q g7xf8=R g7xh8=B g7xh8=N g7xh8=Q g7xh8=R h2-h3 h2-h4 Bc1xb2 Bf1-a6 Bf1-b5 Bf1-c4 Bf1-d3 Bf1-e2 Ke1-e2 Nf3-d4 Nf3-e5 Nf3-g1 Nf3-g5 Nf3-h4 Qd1-e2 Ra1-b1 Rh1-g1");
	test_move_generation( "8/5Rp1/6r1/4p2p/RPp1k2b/4B3/7P/5K2 b - b3 0 1",
						"c4-c3 Bh4-d8 Bh4-e1 Bh4-e7 Bh4-f2 Bh4-f6 Bh4-g3 Bh4-g5 Ke4-d3 Ke4-d5 Ke4xe3 Rg6-a6 Rg6-b6 Rg6-c6 Rg6-d6 Rg6-e6 Rg6-f6 Rg6-g1 Rg6-g2 Rg6-g3 Rg6-g4 Rg6-g5 Rg6-h6");
	test_move_generation( "8/5bk1/8/2Pp4/8/1K6/8/8 w - d6 0 1",
						"c5-c6 Kb3-a2 Kb3-a3 Kb3-a4 Kb3-b2 Kb3-b4 Kb3-c2 Kb3-c3" );

	pass();
}


static void test_move_legality_check( std::string const& fen, move const& m, bool ref_valid )
{
	position p = test_parse_fen( fen );

	check_map check( p );
	bool valid = is_valid_move( p, m, check );
	if( valid != ref_valid ) {
		std::cerr << "Validity mismatch!" << std::endl;
		std::cerr << "Fen: " << fen << std::endl;
		std::cerr << "Move: " << move_to_string(p, m) << std::endl;
		std::cerr << "Reference: " << ref_valid << std::endl;
		std::cerr << "Actual: " << valid << std::endl;
		abort();
	}
}

static void test_move_legality_check()
{
	checking("move legality test");

	test_move_legality_check( "rnbqk2r/pppp1ppp/5n2/2b1p3/4P3/2Q5/PPPP1PPP/RNB1KBNR w KQkq - 0 1", move(square::c3, square::c6, 0), false );
	test_move_legality_check( "rnbqk2r/pppp1ppp/5n2/2b1p3/4P3/2Q5/PPPP1PPP/RNB1KBNR w KQkq - 0 1", move(square::c2, square::c4, 0), false );
	test_move_legality_check( "rnbqk2r/pppp1ppp/5n2/2b1p3/4P3/2Q5/PPPP1PPP/RNB1KBNR w KQkq - 0 1", move(square::c2, square::c3, 0), false );
	test_move_legality_check( "rnbqk2r/pppp1ppp/5n2/2b1p3/4P3/2Q5/PPPP1PPP/RNB1KBNR w KQkq - 0 1", move(square::b2, square::c3, 0), false );
	test_move_legality_check( "rnbqk2r/pppp1ppp/5n2/2b1p3/4P3/2Q5/PPPP1PPP/RNB1KBNR w KQkq - 0 1", move(square::a2, square::b2, 0), false );
	test_move_legality_check( "rnbqk2r/pppp1ppp/5n2/2b1p3/4P3/2Q5/PPPP1PPP/RNB1KBNR w KQkq - 0 1", move(square::a2, square::b5, 0), false );
	test_move_legality_check( "rnbqk2r/pppp1ppp/5n2/2b1p3/4P3/2Q5/PPPP1PPP/RNB1KBNR w KQkq - 0 1", move(square::a2, square::c3, 0), false );
	test_move_legality_check( "8/5bk1/8/2Pp4/8/1K6/8/8 w - d6 0 1", move( square::c5, square::d6, move_flags::enpassant), false );

	pass();
}

static void test_zobrist( std::string const& fen, std::string const& ms )
{
	position p = test_parse_fen( fen );

	move m = test_parse_move( p, ms );

	apply_move( p, m );

	uint64_t new_hash_move = p.hash_;
	p.update_derived();
	uint64_t new_hash_full = p.hash_;

	if( new_hash_move != new_hash_full ) {
		std::cerr << "Hash mismatch: " << std::endl;
		std::cerr << "Fen: " << fen << std::endl;
		std::cerr << "Move: " << ms << std::endl;
		std::cerr << "Full: " << new_hash_full << std::endl;
		std::cerr << "Incremental: " << new_hash_move << std::endl;
		abort();
	}
}

static void test_zobrist()
{
	checking("zobrist hashing");

	test_zobrist( "rnbqk2r/1p3pp1/3bpn2/p2pN2p/P1Pp4/4P3/1P1BBPPP/RN1Q1RK1 b kq c3", "dxc3" );
	test_zobrist( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "e4" );
	test_zobrist( "rn1qkbnr/pppb2pp/5p2/1B1pp3/4P3/2N2N2/PPPP1PPP/R1BQK2R w KQkq e6 0 5", "O-O" );
	test_zobrist( "r3kbnr/pp1n2pp/1qp2p2/3pp2b/4P3/PPN2N1P/2PPBPP1/R1BQR1K1 b kq - 2 10", "O-O-O" );

	pass();
}

static bool test_lazy_eval( std::string const& fen, short& max_difference )
{
	position p = test_parse_fen( fen );

	score currents = p.white() ? p.base_eval : -p.base_eval;
	short current = currents.scale( p.material[0].mg() + p.material[1].mg() );
	short full = evaluate_full( p );

	short diff = std::abs( full - current );

	if( diff > LAZY_EVAL ) {
		std::cerr << "Bug in lazy evaluation, full evaluation differs too much from basic evaluation:" << std::endl;
		std::cerr << "Position:   " << fen << std::endl;
		std::cerr << "Current:    " << current << std::endl;
		std::cerr << "Full:       " << full << std::endl;
		std::cerr << "Difference: " << diff << std::endl;
		return false;
	}

	max_difference = std::max( max_difference, diff );

	return true;
}


static void check_eval()
{
	checking("evaluation");
	if( !eval_values::sane() ) {
		std::cerr << "Evaluation values not sane" << std::endl;
		abort();
	}
	if( eval_values::normalize() ) {
		std::cerr << "Evaluation values not normalized" << std::endl;
		abort();
	}

	pass();
}

static void test_lazy_eval()
{
	checking("lazy evaluation");
	std::ifstream in_fen(conf.self_dir + "test/testpositions.txt");

	short max_difference = 0;

	std::string fen;
	while( std::getline( in_fen, fen ) ) {
		if( !test_lazy_eval( fen, max_difference ) ) {
			abort();
		}
	}

	pass();
	std::cout << "Max positional score difference: " << max_difference << std::endl;
}


static void test_pst() {
	checking("pst symmetry");
	for( int i = 0; i < 64; ++i ) {
		int opposite = (i % 8) + (7 - i / 8) * 8;
		int mirror = (i / 8) * 8 + 7 - i % 8;
		for( int p = 1; p < 7; ++p ) {
			if( pst[0][p][i] != pst[1][p][opposite] ) {
				std::cerr << "PST not symmetric for piece " << p << ", squares " << i << " and " << opposite << ": " << pst[0][p][i] << " " << pst[1][p][opposite] << std::endl;
				abort();
			}
			if( pst[0][p][i] != pst[0][p][mirror] ) {
				std::cerr << "PST not symmetric for piece " << p << ", squares " << i << " and " << mirror << ": " << pst[0][p][i] << " " << pst[0][p][mirror] << std::endl;
				abort();
			}
		}
	}
	pass();
}


std::string flip_fen( std::string const& fen )
{
	std::string flipped;

	std::string remaining;
	std::string first_part = split( fen, remaining );
	first_part += '/';
	while( !first_part.empty() ) {
		std::string segment = split( first_part, first_part, '/' );
		if( !flipped.empty() ) {
			flipped = '/' + flipped;
		}
		flipped = segment + flipped;
	}

	flipped += " ";
	if( remaining[0] == 'b' ) {
		flipped += "w";
	}
	else {
		flipped += "b";
	}

	if( remaining[remaining.size() - 1] == '6' ) {
		remaining[remaining.size() - 1] = '3';
	}
	else if( remaining[remaining.size() - 1] == '3' ) {
		remaining[remaining.size() - 1] = '6';
	}
	flipped += remaining.substr(1);

	for( std::size_t i = 0; i < flipped.size(); ++i ) {
		switch( flipped[i] ) {
		case 'p':
			flipped[i] = 'P';
			break;
		case 'n':
			flipped[i] = 'N';
			break;
		case 'b':
			flipped[i] = 'B';
			break;
		case 'r':
			flipped[i] = 'R';
			break;
		case 'q':
			flipped[i] = 'Q';
			break;
		case 'k':
			flipped[i] = 'K';
			break;
		case 'P':
			flipped[i] = 'p';
			break;
		case 'N':
			flipped[i] = 'n';
			break;
		case 'B':
			flipped[i] = 'b';
			break;
		case 'R':
			flipped[i] = 'r';
			break;
		case 'Q':
			flipped[i] = 'q';
			break;
		case 'K':
			flipped[i] = 'k';
			break;
		default:
			break;
		}
	}

	return flipped;
}


static bool test_evaluation( std::string const& fen, position const& p )
{
	std::string flipped = flip_fen( fen );

	position p2 = test_parse_fen( flipped );

	if( p.material[0] != p2.material[1] || p.material[1] != p2.material[0] ) {
		std::cerr << "Material not symmetric: " << p.material[0] << " " << p.material[1] << " " << p2.material[0] << " " << p2.material[1] << std::endl;
		std::cerr << "Fen: " << fen << std::endl;
		std::cerr << "Flipped: " << flipped << std::endl;
		return false;
	}

	if( p.base_eval.mg() != -p2.base_eval.mg() ||
		p.base_eval.eg() != -p2.base_eval.eg() )
	{
		std::cerr << "Base evaluation not symmetric: " << p.base_eval << " " << p2.base_eval << std::endl;
		std::cerr << "Fen: " << fen << std::endl;
		std::cerr << "Flipped: " << flipped << std::endl;
		return false;
	}

	short eval_full = evaluate_full( p );
	short flipped_eval_full = evaluate_full( p2 );
	if( eval_full != flipped_eval_full ) {
		std::cerr << "Evaluation not symmetric: " << eval_full << " " << flipped_eval_full << " " << std::endl;
		std::cerr << "Fen: " << fen << std::endl;
		std::cerr << explain_eval( p ) << std::endl;
		std::cerr << "Flipped: " << flipped << std::endl;
		std::cerr << explain_eval( p2 ) << std::endl;
		return false;
	}


	return true;
}


static void test_moves_noncaptures( std::string const& fen, position const& p )
{
	check_map check( p );
	if( check.check ) {
		return;
	}

	auto noncaptures = calculate_moves<movegen_type::noncapture>( p, check );
	auto pseudocheck = calculate_moves<movegen_type::pseudocheck>( p, check );
	
	for( auto m : noncaptures ) {
		position p2 = p;
		apply_move( p2, m );
		check_map new_check( p2 );

		auto it = pseudocheck.begin();
		for( ; it != pseudocheck.end(); ++it ) {
			if( *it == m ) {
				break;
			}
		}

		if( new_check.check && it == pseudocheck.end() ) {
			std::cerr << "Checking move not generated by calculate_moves_noncaptures<true>" << std::endl;
			std::cerr << "Fen: " << fen << std::endl;
			std::cerr << "Move: " << move_to_string( p, m ) << std::endl;
			abort();
		}
		
		if( it != pseudocheck.end() ) {
			pseudocheck.erase( it );
		}
	}

	if( !pseudocheck.empty() ) {
		abort();
	}	
}


static void test_incorrect_positions()
{
	checking("fen validation");
	std::ifstream in_fen(conf.self_dir + "test/bad_fen.txt");

	std::string fen;
	while( std::getline( in_fen, fen ) ) {
		position p;
		std::string error;
		if( parse_fen( fen, p, &error ) ) {
			std::cerr << "Fen of invalid position was not rejected." << std::endl;
			std::cerr << "Fen: " << fen << std::endl;
			abort();
		}
	}
	pass();
}


static void process_test_positions()
{
	std::ifstream in_fen(conf.self_dir + "test/testpositions.txt");

	std::string fen;
	while( std::getline( in_fen, fen ) ) {
		position p = test_parse_fen( fen );

		if( !test_evaluation( fen, p ) ) {
			abort();
		}

		test_moves_noncaptures( fen, p );
	}
}


static void test_perft()
{
	checking( "perft", true );

	{
		perft<true>(6);
		perft<false>(6);
	}
	{
		position p = test_parse_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -");
		uint64_t const perft_results[] = {
			6,
			264,
			9467,
			422333,
			15833292,
			706045033
		};

		perft<true>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
		uint64_t const perft_results[] = {
			14,
			191,
			2812,
			43238,
			674624,
			11030083,
			178633661
		};

		perft<true>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
		uint64_t const perft_results[] = {
			48,
			2039,
			97862,
			4085603,
			193690690
		};

		perft<true>(p, 5, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 5, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("rnbqkb1r/pp1p1ppp/2p5/4P3/2B5/8/PPP1NnPP/RNBQK2R w KQkq - 0 6");
		uint64_t const perft_results[] = {
			42,
			1352,
			53392
		};

		perft<true>(p, 3, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 3, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("8/5bk1/8/2Pp4/8/1K6/8/8 w - d6 0 1");
		uint64_t const perft_results[] = {
			8,
			104,
			736,
			9287,
			62297,
			824064
		};

		perft<true>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1");
		uint64_t const perft_results[] = {
			15,
			126,
			1928,
			13931,
			206379,
			1440467
		};

		perft<true>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("5k2/8/8/8/8/8/8/4K2R w K - 0 1");
		uint64_t const perft_results[] = {
			15,
			66,
			1198,
			6399,
			120330,
			661072
		};

		perft<true>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1");
		uint64_t const perft_results[] = {
			16,
			71,
			1286,
			7418,
			141077,
			803711 
		};

		perft<true>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1");
		uint64_t const perft_results[] = {
			26,
			1141,
			27826,
			1274206
		};

		perft<true>(p, 4, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 4, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1");
		uint64_t const perft_results[] = {
			44,
			1494,
			50509,
			1720476
		};

		perft<true>(p, 4, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 4, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1");
		uint64_t const perft_results[] = {
			11,
			133,
			1442,
			19174,
			266199,
			3821001
		};

		perft<true>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1");
		uint64_t const perft_results[] = {
			29,
			165,
			5160,
			31961,
			1004658
		};

		perft<true>(p, 5, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 5, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("4k3/1P6/8/8/8/8/K7/8 w - - 0 1");
		uint64_t const perft_results[] = {
			9,
			40,
			472,
			2661,
			38983,
			217342
		};

		perft<true>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("8/P1k5/K7/8/8/8/8/8 w - - 0 1");
		uint64_t const perft_results[] = {
			6,
			27,
			273,
			1329,
			18135,
			92683
		};

		perft<true>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("K1k5/8/P7/8/8/8/8/8 w - - 0 1");
		uint64_t const perft_results[] = {
			2,
			6,
			13,
			63,
			382,
			2217
		};

		perft<true>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 6, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("8/k1P5/8/1K6/8/8/8/8 w - - 0 1");
		uint64_t const perft_results[] = {
			10,
			25,
			268,
			926,
			10857,
			43261,
			567584
		};

		perft<true>(p, 7, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 7, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
	{
		position p = test_parse_fen("8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1");
		uint64_t const perft_results[] = {
			37,
			183,
			6559,
			23527 
		};

		perft<true>(p, 4, perft_results, sizeof(perft_results) / sizeof(uint64_t));
		perft<false>(p, 4, perft_results, sizeof(perft_results) / sizeof(uint64_t));
	}
}

static void do_check_popcount( uint64_t v, uint64_t expected )
{
	uint64_t c = popcount(v);
	if( c != expected ) {
		std::cerr << "Popcount failed on " << v << ", got " << c << ", expected " << expected << std::endl;
		abort();
	}
}

static void check_popcount()
{
	checking("popcount");
	for( unsigned int i = 0; i < 64; ++i ) {
		uint64_t v = 1ull << i;
		do_check_popcount( v, 1 );

		do_check_popcount( v - 1, i );
	}
	pass();
}

static void do_check_bitscan( uint64_t v, uint64_t expected_count, uint64_t expected_sum, uint64_t expected_reverse_sum )
{
	uint64_t c = 0;
	uint64_t sum = 0;
	uint64_t v2 = v;

	while( v2 ) {
		++c;
		uint64_t i = bitscan_unset( v2 );
		sum += i * c;
	}
	if( c != expected_count || sum != expected_sum ) {
		std::cerr << "Bitscan failed on " << v << ", got " << c << ", " << sum << ", expected " << expected_count << ", " << expected_sum << std::endl;
		abort();
	}

	c = 0;
	sum = 0;
	v2 = v;
	while( v2 ) {
		++c;
		uint64_t i = bitscan_reverse( v2 );
		v2 ^= 1ull << i;
		sum += i * c;
	}
	if( c != expected_count || sum != expected_reverse_sum ) {
		std::cerr << "Reverse bitscan failed on " << v << ", got " << c << ", " << sum << ", expected " << expected_count << ", " << expected_reverse_sum << std::endl;
		abort();
	}
}

static void check_bitscan()
{
	checking("bitscan");
	do_check_bitscan( 0x5555555555555555ull, 32, 21824, 10912 );
	pass();
}

static void check_see( std::string const& fen, std::string const& ms, int expected )
{
	position p = test_parse_fen( fen );

	move m = test_parse_move( p, ms );
	
	short v = see( p, m );
	if( v > 0 && expected <= 0 ) {
		std::cerr << "See of " << fen << " " << ms << " is " << v << ", expected type " << expected << std::endl;
		abort();
	}
	if( v < 0 && expected >= 0 ) {
		std::cerr << "See of " << fen << " " << ms << " is " << v << ", expected type " << expected << std::endl;
		abort();
	}
	if( v == 0 && expected != 0 ) {
		std::cerr << "See of " << fen << " " << ms << " is " << v << ", expected type " << expected << std::endl;
		abort();
	}
}

static void check_see()
{
	checking("static exchange evaluation");

	check_see( "r3r3/p4ppp/4k3/R3n3/1N1KP3/8/6BP/8 w - -", "Rxe5", 1 );
	check_see( "8/1k6/3r4/8/3Pp3/8/6K1/3R4 b - d3 0 1", "exd", 1 );
	check_see( "8/1k6/3b4/8/3Pp3/8/6K1/3R4 b - d3 0 1", "exd", 0 );
	check_see( "1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - -", "Rxe5", 1 );
	check_see( "1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - -", "Nxe5", -1 );

	pass();
}


void do_check_disambiguation( std::string const& fen, std::string const& ms, std::string const& ref )
{
	position p = test_parse_fen( fen );

	move m;
	std::string error;
	if( !parse_move( p, ms, m, error ) ) {
		std::cerr << error << ": " << ms << std::endl;
		abort();
	}

	std::string const san = move_to_san( p, m );
	if( san != ref ) {
		std::cerr << "Could not obtain SAN of move." << std::endl;
		std::cerr << "Actual: " << san << std::endl;
		std::cerr << "Expected: " << ref << std::endl;
		std::cerr << "Fen: " << fen << std::endl;
		abort();
	}
}


void check_disambiguation()
{
	checking("disambiguations");
	do_check_disambiguation( "2K5/8/8/4N3/8/8/8/2k3N1 w - - 0 1", "Ngf3", "Ngf3" );
	do_check_disambiguation( "2K5/8/8/4N3/8/8/8/2k3N1 w - - 0 1", "N1f3", "Ngf3" );
	do_check_disambiguation( "2K5/8/8/4N3/8/8/8/2k1N3 w - - 0 1", "Ne1f3", "N1f3" );
	do_check_disambiguation( "2K5/8/8/4N3/8/8/8/2k1N3 w - - 0 1", "N1f3", "N1f3" );
	do_check_disambiguation( "2K5/8/8/4N3/8/8/8/2k1N1N1 w - - 0 1", "Ne1f3", "Ne1f3" );
	do_check_disambiguation( "2K5/8/8/4N3/8/8/8/2k1N1N1 w - - 0 1", "Ne5f3", "N5f3" );
	pass();
}


void check_condition_wait()
{
	checking("condition wait");
	mutex m;
	scoped_lock l(m);
	condition c;

	for( int i = 100; i <= 1000; i += 100 ) {
		duration wait = duration::milliseconds(i);
		timestamp const start;
		c.wait(l, wait);
		timestamp const stop;

		duration elapsed = stop - start;

		if( elapsed > (wait + duration::milliseconds(1)) || elapsed < wait ) {
			std::cerr << "Expected to wait: " << wait.milliseconds() << " ms" << std::endl;
			std::cerr << "Actually waited:  " << elapsed.milliseconds() << " ms" << std::endl;
#if UNIX
			if( elapsed < wait ) {
				std::cerr << "If there has been a leap second since last reboot, update your kernel and reboot!." << std::endl;
			}
#endif
			abort();
		}
	}
	pass();
}


void check_kpvk( std::string const& fen, bool evaluated, int expected )
{
	position p = test_parse_fen( fen );

	short s;
	if( evaluate_endgame( p, s ) != evaluated ) {
		abort();
	}
	if( evaluated ) {
		if( !expected && s != result::draw ) {
			abort();
		}
		else if( expected > 0 && s < result::win_threshold ) {
			abort();
		}
		else if( expected < 0 && s >= result::loss_threshold ) {
			abort();
		}
	}

	position p2 = test_parse_fen( flip_fen( fen ) );
	expected *= -1;

	if( evaluate_endgame( p2, s ) != evaluated ) {
		abort();
	}

	if( evaluated ) {
		if( !expected && s != result::draw ) {
			abort();
		}
		else if( expected > 0 && s < result::win_threshold ) {
			abort();
		}
		else if( expected < 0 && s >= result::loss_threshold ) {
			abort();
		}
	}
}


void check_endgame_eval()
{
	checking("endgame evaluation");

	check_kpvk("k7/8/7p/8/8/8/8/1K6 w - - 1 1", false, 0 );
	check_kpvk("k7/8/7p/8/8/8/8/1K6 b - - 1 1", true, -1 );
	check_kpvk("k7/7K/7p/8/8/8/8/8 b - - 1 1", true, -1 );
	check_kpvk("k7/7K/7p/8/8/8/8/8 w - - 1 1", false, 0 );
	check_kpvk("k7/8/2K5/7p/8/8/8/8 b - - 1 1", true, -1 );
	check_kpvk("k7/8/2K5/7p/8/8/8/8 w - - 1 1", false, 0 );

	std::string const fens[] = {
		"8/3k4/8/8/5K2/8/8/8 w - - 0 1",
		"8/8/2k5/4n1K1/8/8/8/8 w - - 0 1",
		"8/6B1/8/8/8/7K/2k5/8 w - - 0 1",
		"5B2/8/8/2n5/8/8/2k4K/8 w - - 0 1",
		"5b2/8/8/2B5/8/8/2k4K/8 w - - 0 1",
		"4Q3/8/8/2q5/8/8/2k4K/8 w - - 0 1",
		"8/5K2/6R1/2r5/1k6/8/8/8 w - - 0 1",
		"8/5K2/6R1/8/1k6/8/3b4/8 w - - 0 1",
		"8/5K2/6R1/8/1k6/8/4n3/8 w - - 0 1",
		"r7/5K2/6R1/8/1k6/8/4n3/8 w - - 0 1",
		"r7/5K2/6R1/8/1k6/8/1b6/8 w - - 0 1",
		"3n4/K7/8/6Q1/4k3/8/8/5n2 w - - 0 1",
		"8/8/8/6Q1/K3k3/2q5/8/5b2 w - - 0 1",
		"8/8/8/6Q1/K3k3/8/7b/5b2 w - - 0 1",
		"8/8/8/6Q1/K3k3/8/7n/5q2 w - - 0 1",
		"8/8/3k4/6P1/8/2K5/7n/8 w - - 0 1",
		"1b6/8/8/6P1/4k3/2K5/8/8 w - - 0 1",
		"8/7k/8/8/8/7P/7K/8 w - - 0 1",
		"8/7k/8/8/8/1B5P/7K/8 w - - 0 1",
		"8/2b4k/8/8/3P4/3K4/2B5/8 w - - 0 1",
		"8/7k/8/8/3N4/3K4/2B5/8 w - - 0 1",
		"7k/8/8/1p6/8/8/8/6K1 b - - 0 1",
		"7k/8/8/1p6/8/8/8/7K w - - 0 1"
	};

	for( auto const& fen: fens ) {
		position p = test_parse_fen( fen );
		position p2 = test_parse_fen( flip_fen( fen ) );

		short ev = 0;
		if( !evaluate_endgame( p, ev ) ) {
			std::cerr << "Not recognized as endgame: " << fen << std::endl;
			abort();
		}
		short ev2 = 0;
		if( !evaluate_endgame( p2, ev2 ) ) {
			std::cerr << "Not recognized as endgame: " << flip_fen( fen ) << std::endl;
			abort();
		}

		if( ev != -ev2 ) {
			std::cerr << "Difference in endgame evaluation: " << ev << " " << ev2 << std::endl;
			std::cerr << "Fen: " << fen << std::endl;
			abort();
		}
	}
	pass();
}

void check_time()
{
	checking("time classes");
	timestamp t;
	timestamp t2;
	timestamp t3 = t2;
	usleep(2000);
	timestamp t4;

	if( t > t2 ) {
		std::cerr << "Timestamp error: t > t2" << std::endl;
		abort();
	}
	if( t2 != t3 ) {
		std::cerr << "Timestamp error: t2 != t3" << std::endl;
		abort();
	}
	if( t4 <= t3 ) {
		std::cerr << "Timestamp error: t4 <= t3" << std::endl;
		abort();
	}
	if( t3 >= t4 ) {
		std::cerr << "Timestamp error: t3 >= t4" << std::endl;
		abort();
	}


	duration d( duration::milliseconds(100) );

	if( d.seconds() != 0 ) {
		std::cerr << "duration::milliseconds(100).seconds() != 0" << std::endl;
		abort();
	}
	if( d.nanoseconds() != 100000000 ) {
		std::cerr << "duration::milliseconds(100).nanoseconds() != 100000000" << std::endl;
		abort();
	}
	pass();
}

void check_scale()
{
	checking("checking smoothness of game phase scaling");

	score v(20, 20);
	score v2(618, 598);
	for( int i = eval_values::phase_transition_material_begin; i >= eval_values::phase_transition_material_end; --i ) {
		short scaled = v.scale( i );

		if( scaled != 20 ) {
			std::cerr << "Scaling did not return correct value" << std::endl;
			abort();
		}

		short scaled2 = v2.scale( i );
		short nscaled2 = (-v2).scale( i );
		if( scaled2 != -nscaled2 ) {
			v2.scale(i);
			(-v2).scale(i);
			std::cerr << "Scaling of negated score did not return negated value" << std::endl;
			abort();
		}
	}

	pass();
}
}

bool selftest()
{
	check_popcount();
	check_bitscan();
	check_see();

	pawn_hash_table.init( conf.pawn_hash_table_size() );

	check_disambiguation();

	check_time();

	check_scale();
	check_eval();
	check_endgame_eval();

	test_pst();
	test_incorrect_positions();
	process_test_positions();
	test_move_generation();
	test_move_legality_check();
	test_zobrist();
	test_lazy_eval();

	check_condition_wait();

	test_perft();

	std::cerr << "Self test passed" << std::endl;
	return true;
}
