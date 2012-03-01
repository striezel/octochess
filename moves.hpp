#ifndef __MOVES_H__
#define __MOVES_H__

#include "chess.hpp"
#include "detect_check.hpp"

class killer_moves;

struct move_info {
	move m;
	short evaluation;
	int sort;
	position::pawn_structure pawns;
};

// Calculates all legal moves
// Returned evaluation is fast_eval
void calculate_moves( position const& p, color::type c, move_info*& moves, check_map const& check );

// Returns all legal captures
// Precondition: Own king not in check
// Returned evaluation is MVV/LVA
void calculate_moves_captures( position const& p, color::type c, move_info*& moves, check_map const& check );

// Calculates all legal non-captures
// Returned evaluation is fast_eval
void calculate_moves_noncaptures( position const& p, color::type c, move_info*& moves, check_map const& check );

#endif
