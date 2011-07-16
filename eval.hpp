#ifndef __EVAL_H__
#define __EVAL_H__

#include "chess.hpp"

int evaluate( position const& p, color::type c );

int evaluate_move( position const& p, color::type c, int current_evaluation, move const& m );

#endif
