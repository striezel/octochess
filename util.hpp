#ifndef __UTIL_H__
#define __UTIL_H__

#include "chess.hpp"

#include <string>

struct move_info;

bool validate_move( position const& p, move const& m, color::type c );
bool parse_move( position& p, color::type c, std::string const& line, move& m );

std::string move_to_string( position const& p, color::type c, move const& m );

void init_board( position& p );

bool apply_move( position& p, move const& m, color::type c, bool& capture );
bool apply_move( position& p, move_info const& m, color::type c, bool& capture );


void init_random( unsigned long long seed );

unsigned char get_random_unsigned_char();

unsigned long long get_random_unsigned_long_long();

#endif
