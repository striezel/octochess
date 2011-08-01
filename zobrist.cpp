#include "zobrist.hpp"
#include "chess.hpp"
#include "util.hpp"

namespace {
static unsigned long long data[2][16][8][8];
unsigned long long white_to_move;
unsigned long long enpassant[8];

unsigned long long promoted_pawns[2][8];
unsigned long long can_castle[2][2];
unsigned long long castled[2];
}

void init_zobrist_tables()
{
	white_to_move = get_random_unsigned_long_long();

	for( unsigned int c = 0; c < 2; ++c ) {
		for( unsigned int pi = 0; pi < 16; ++pi ) {
			for( unsigned int col = 0; col < 8; ++col ) {
				for( unsigned int row = 0; row < 8; ++row ) {
					data[c][pi][col][row] = get_random_unsigned_long_long();
				}
			}
		}
	}

	for( unsigned int c = 0; c < 2; ++c ) {
		for( unsigned int pi = 0; pi < 8; ++pi ) {
			promoted_pawns[c][pi] = get_random_unsigned_long_long();
		}
		can_castle[c][0] = get_random_unsigned_long_long();
		can_castle[c][1] = get_random_unsigned_long_long();
		castled[c] = get_random_unsigned_long_long();
	}

	for( unsigned int i = 0; i < 8; ++i ) {
		enpassant[i] = get_random_unsigned_long_long();
	}
}

unsigned long long get_zobrist_hash( position const& p, color::type c ) {
	unsigned long long ret = c ? 0 : white_to_move;

	for( unsigned int c = 0; c < 2; ++c ) {
		for( unsigned int pi = 0; pi < 8; ++pi ) {
			piece const& pp = p.pieces[c][pi];
			if( pp.alive ) {
				ret ^= data[c][pi][pp.column][pp.row];
				if( pp.special ) {
					ret ^= promoted_pawns[c][pi];
				}
			}
		}
		for( unsigned int pi = 8; pi < 16; ++pi ) {
			piece const& pp = p.pieces[c][pi];
			if( pp.alive ) {
				ret ^= data[c][pi][pp.column][pp.row];
			}
		}

		{
			piece const& pp = p.pieces[c][pieces::rook1];
			if( pp.alive && pp.special ) {
				ret ^= can_castle[c][0];
			}
		}
		{
			piece const& pp = p.pieces[c][pieces::rook2];
			if( pp.alive && pp.special ) {
				ret ^= can_castle[c][1];
			}
		}

		{
			piece const& pp = p.pieces[c][pieces::king];
			if( pp.special ) {
				ret ^= castled[c];
			}
		}
	}

	if( p.can_en_passant != pieces::nil ) {
		ret ^= enpassant[p.can_en_passant];
	}

	return ret;
}

namespace {
static void subtract_target( position const& p, color::type c, unsigned long long& hash, int target, int col, int row )
{
	if( target >= pieces::pawn1 && target <= pieces::pawn8 ) {
		piece const& pp = p.pieces[1-c][target];
		if( pp.special ) {
			hash ^= promoted_pawns[1-c][target];
		}
	}
	else if( target == pieces::rook1 || target == pieces::rook2 ) {
		piece const& pp = p.pieces[1-c][target];
		if( pp.special ) {
			hash ^= can_castle[1-c][target - pieces::rook1];
		}
	}
	hash ^= data[1-c][target][col][row];
}
}

unsigned long long update_zobrist_hash( position const& p, color::type c, unsigned long long hash, move const& m )
{
	if( p.can_en_passant != pieces::nil ) {
		hash ^= enpassant[p.can_en_passant];
	}

	int target = p.board[m.target_col][m.target_row];
	if( target != pieces::nil ) {
		target &= 0x0f;
		subtract_target( p, c, hash, target, m.target_col, m.target_row );
	}

	piece const& pp = p.pieces[c][m.piece];
	if( m.piece >= pieces::pawn1 && m.piece <= pieces::pawn8 ) {
		if( m.target_col != pp.column && target == pieces::nil ) {
			// Was en-passant
			hash ^= data[1-c][p.board[m.target_col][pp.row] & 0x0f][m.target_col][pp.row];
		}
		else if( (m.target_row == 0 || m.target_row == 7) && !pp.special ) {
			// Promition
			hash ^= promoted_pawns[c][m.piece];
		}
		else if( m.target_row == pp.row + 2 || m.target_row + 2 == pp.row ) {
			// Becomes en-passantable
			hash ^= enpassant[m.piece];
		}
	}
	else if( m.piece == pieces::rook1 || m.piece == pieces::rook2 ) {
		if( pp.special ) {
			hash ^= can_castle[c][m.piece - pieces::rook1];
		}
	}
	else if( m.piece == pieces::king ) {
		if( (pp.column == m.target_col + 2) || (pp.column + 2 == m.target_col) ) {
			// Was castling
			if( m.target_col == 2 ) {
				hash ^= data[c][pieces::rook1][0][m.target_row];
				hash ^= data[c][pieces::rook1][3][m.target_row];
			}
			else {
				hash ^= data[c][pieces::rook2][7][m.target_row];
				hash ^= data[c][pieces::rook2][5][m.target_row];
			}
			hash ^= castled[c];
		}
		piece const& r1 = p.pieces[c][pieces::rook1];
		if( r1.alive && r1.special ) {
			hash ^= can_castle[c][0];
		}
		piece const& r2 = p.pieces[c][pieces::rook2];
		if( r2.alive && r2.special ) {
			hash ^= can_castle[c][1];
		}
	}
	hash ^= data[c][m.piece][pp.column][pp.row];
	hash ^= data[c][m.piece][m.target_col][m.target_row];

	hash ^= white_to_move;

	return hash;
}