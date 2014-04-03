//
//  SearchHandler.h
//  Connect-r
//
//  Created by Jonathan Howard on 3/31/14.
//  Copyright (c) 2014 Jonathan Howard. All rights reserved.
//

#ifndef __Connect_r__SearchHandler__
#define __Connect_r__SearchHandler__

#include <iostream>
#include "GameState.h"
#include "HeuristicScores.h"

move_t findNextMove(Game *g);
int negaMax(Game * g, int depth, int alpha, int beta);

#endif /* defined(__Connect_r__SearchHandler__) */
