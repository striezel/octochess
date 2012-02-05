#include "fen.hpp"
#include "util.hpp"
#include "tables.hpp"

#include <iostream>
#include <sstream>

std::string position_to_fen_noclock( position const& p, color::type c )
{
	std::stringstream ss;

	for( int row = 7; row >= 0; --row ) {
		int free = 0;

		for( int col = 0; col < 8; ++col ) {
			int pi = row * 8 + col;

			pieces::type piece;
			color::type c;
			if( p.bitboards[color::black].b[bb_type::all_pieces] & (1ull << pi) ) {
				c = color::black;
				piece = get_piece_on_square( p, c, pi );
			}
			else {
				c = color::white;
				piece = get_piece_on_square( p, c, pi );
			}

			if( free ) {
				ss << free;
				free = 0;
			}

			unsigned char name;

			switch( piece ) {
			default:
			case pieces::king:
				name = 'K';
				break;
			case pieces::queen:
				name = 'Q';
				break;
			case pieces::rook:
				name = 'R';
				break;
			case pieces::bishop:
				name = 'B';
				break;
			case pieces::knight:
				name = 'N';
				break;
			case pieces::pawn:
				name = 'P';
				break;
			}

			if( c == color::black ) {
				name += 'a' - 'A';
			}

			ss << name;
		}

		if( free ) {
			ss << free;
		}

		if( row ) {
			ss << '/';
		}
	}

	if( c ) {
		ss << " b ";
	}
	else {
		ss << " w ";
	}

	bool can_castle = false;
	if( p.castle[0] & 0x1 ) {
		can_castle = true;
		ss << 'K';
	}
	if( p.castle[0] & 0x2 ) {
		can_castle = true;
		ss << 'Q';
	}
	if( p.castle[1] & 0x1 ) {
		can_castle = true;
		ss << 'k';
	}
	if( p.castle[1] & 0x2 ) {
		can_castle = true;
		ss << 'q';
	}

	if( !can_castle ) {
		ss << '-';
	}

	ss << ' ';

	if( p.can_en_passant ) {
		ss << static_cast<unsigned char>('a' + (p.can_en_passant % 8));
		ss << static_cast<unsigned char>('1' + (p.can_en_passant / 8));
	}
	else {
		ss << '-';
	}

	position p2;
	color::type c2;
	if( !parse_fen_noclock( ss.str(), p2, c2 ) ) {
		std::cerr << "FAIL" << std::endl;
	}
	return ss.str();
}

bool parse_fen_noclock( std::string const& fen, position& p, color::type& c )
{
	std::stringstream in( fen );

	std::string board;
	std::string color;
	std::string castle;
	std::string enpassant;

	in >> board >> color >> castle >> enpassant;

	if( !in ) {
		return false;
	}

	if( color != "w" && color != "b" ) {
		return false;
	}
	if( color == "w" ) {
		c = color::white;
	}
	else {
		c = color::black;
	}

	p.castle[0] = 0;
	p.castle[1] = 0;
	if( castle != "-" ) {
		for( std::size_t i = 0; i < castle.size(); ++i ) {
			switch( castle[i] ) {
			case 'Q':
				p.castle[0] |= 0x2;
				break;
			case 'K':
				p.castle[0] |= 0x1;
				break;
			case 'q':
				p.castle[1] |= 0x2;
				break;
			case 'k':
				p.castle[1] |= 0x1;
				break;
			default:
				return false;
			}
		}
	}

	p.can_en_passant = 0;

	if( enpassant != "-" ) {
		if( enpassant.size() != 2 ) {
			return false;
		}
		if( enpassant[0] < 'a' || enpassant[0] > 'h' || enpassant[1] < '1' || enpassant[1] > '8' ) {
			return false;
		}

		p.can_en_passant = (enpassant[1] - '1') * 8 + enpassant[0] - 'a';
	}

	p.clear_bitboards();

	int row = 7;
	int col = 0;
	bool last_was_free = false;
	const char* cur = board.c_str();
	while( *cur ) {
		if( row < 0 ) {
			return false;
		}

		if( *cur == '/' ) {
			if( col != 8 ) {
				return false;
			}
			--row;
			col = 0;
			++cur;
			last_was_free = false;
			continue;
		}

		if( col >= 8 ) {
			return false;
		}

		if( *cur >= '1' && *cur <= '8') {
			if( last_was_free ) {
				return false;
			}
			col += *cur - '0';
			last_was_free = true;
		}
		else {
			last_was_free = false;

			unsigned char pi = col + row * 8;

			pieces::type piece;
			color::type c;
			switch( *cur ) {
			default:
				return false;
			case 'p':
				c = color::black;
				piece = pieces::pawn;
				break;
			case 'n':
				c = color::black;
				piece = pieces::knight;
				break;
			case 'b':
				c = color::black;
				piece = pieces::bishop;
				break;
			case 'r':
				c = color::black;
				piece = pieces::rook;
				break;
			case 'q':
				c = color::black;
				piece = pieces::queen;
				break;
			case 'k':
				c = color::black;
				piece = pieces::king;
				break;
			case 'P':
				c = color::white;
				piece = pieces::pawn;
				break;
			case 'N':
				c = color::white;
				piece = pieces::knight;
				break;
			case 'B':
				c = color::white;
				piece = pieces::bishop;
				break;
			case 'R':
				c = color::white;
				piece = pieces::rook;
				break;
			case 'Q':
				c = color::white;
				piece = pieces::queen;
				break;
			case 'K':
				c = color::white;
				piece = pieces::king;
				break;
			}
			p.bitboards[c].b[piece] |= 1ull << pi;
			++col;
		}


		++cur;
	}

	for( int c = 0; c < 2; ++c ) {
		p.bitboards[c].b[bb_type::all_pieces] = p.bitboards[c].b[bb_type::pawns] | p.bitboards[c].b[bb_type::knights] | p.bitboards[c].b[bb_type::bishops] | p.bitboards[c].b[bb_type::rooks] | p.bitboards[c].b[bb_type::queens] | p.bitboards[c].b[bb_type::king];

		p.bitboards[c].b[bb_type::pawn_control] = 0;
		unsigned long long pawn;
		unsigned long long pawns = p.bitboards[c].b[bb_type::pawns];
		while( pawns ) {
			bitscan( pawns, pawn );
			pawns &= pawns - 1;
			p.bitboards[c].b[bb_type::pawn_control] |= pawn_control[c][pawn];
		}
	}

	init_material( p );
	p.init_pawn_structure();

	return true;
}
