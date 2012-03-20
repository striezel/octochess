#ifndef __CALC_H__
#define __CALC_H__

#include "config.hpp"
#include "chess.hpp"
#include "detect_check.hpp"
#include "pvlist.hpp"
#include "moves.hpp"
#include "seen_positions.hpp"

int const depth_factor = 6;

struct new_best_move_callback
{
	virtual void on_new_best_move( position const& p, color::type c, int depth, int evaluation, uint64_t nodes, pv_entry const* pv );
};
extern new_best_move_callback default_new_best_move_callback;

class calc_manager
{
public:
	calc_manager();
	virtual ~calc_manager();

	// May modify seen_positions at indexes > root_position
	bool calc( position& p, color::type c, move& m, int& res,
		   duration const& move_time_limit, int clock,
		   seen_positions& seen, short last_mate,
		   new_best_move_callback& new_best_cb = default_new_best_move_callback );

private:
	class impl;
	impl* impl_;
};

class killer_moves
{
public:
	killer_moves() {}

	void add_killer( move const& m ) {
		if( m1 != m ) {
			m2 = m1;
			m1 = m;
		}
	}

	bool is_killer( move const& m ) const {
		return m == m1 || m == m2;
	}

	move m1;
	move m2;
};

class context
{
public:
	context()
		: clock(0)
		, move_ptr(moves)
	{
	}

	unsigned char clock; // The halfmove clock
	pv_entry_pool pv_pool;

	move_info moves[200 * (MAX_DEPTH + MAX_QDEPTH)];
	move_info* move_ptr;

	seen_positions seen;

	killer_moves killers[2][MAX_DEPTH + 1];
};

// Depth is number of plies to search multiplied by depth_factor
short step( int depth, int ply, context& ctx, position const& p, uint64_t hash, color::type c, check_map const& check, short alpha, short beta, pv_entry* pv, bool last_was_null );

#endif
