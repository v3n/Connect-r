 //
//  SearchHandler.cpp
//  Connect-r
//
//  Created by Jonathan Howard on 3/31/14.
//  Copyright (c) 2014 Jonathan Howard. All rights reserved.
//

#include <vector>
#include "SearchHandler.h"
#include <dispatch/dispatch.h>
#include <climits>

dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
bool shouldAbort = false;

move_t findNextMove(Game * g) {
    __block struct {
        __block int score[WIDTH];
    } scores;
    
    for(int i = 0; i < WIDTH; i++)
        scores.score[i] = -INT_MIN;
    
    dispatch_group_t group = dispatch_group_create();
    Game * currentGame;
    
    std::vector<move_t> moves = g->getOrderedMoves();
    
    for(std::vector<move_t>::iterator i = moves.begin(); i != moves.end(); ++i) {
        if(g->move(*i)) {
            currentGame = new Game((const Game)*g);
            g->unmove();
            
            dispatch_group_async(group, queue, ^{
                scores.score[*i] = negaMax(currentGame, HORIZON, -H_MAX, H_MAX);
            });
        }
        currentGame = nullptr;
    }
    
    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    
    //FIXME: terminating early corrupts the data. need to determine why this is.
//      dispatch_group_wait(group, dispatch_time(DISPATCH_TIME_NOW, 2LL * NSEC_PER_SEC));
    
    //TODO: Randomly select best move (if there are ties) instead of picking the last one we look at.
    move_t v = 0;
    for(int i = 0; i < WIDTH; i++) {
        if(scores.score[i] >= scores.score[v])
            v = i;
    }
    dispatch_release(group);
    
    return v;
}

//I chose negaMax because the implementation for a zero-sum
//game is simpler than minimax. This is implemented as a C-style function
//for ease of use in Grand Central Dispatch.
int negaMax(Game * g, int depth, int alpha, int beta) {
    int value;
    if (depth == 0 || BOARDSIZE - g->nMoves == 0) {
        return g->rate();
    }
    
    std::vector<move_t> moves = g->getOrderedMoves();
    int best = -H_MAX;
    
    for(std::vector<move_t>::iterator i = moves.begin(); i != moves.end(); ++i) {
        g->move(*i);
        value = -negaMax(g, depth-1, -beta, -alpha);
        g->unmove();
        
        if(value > best)
            best = value;
        if(best > alpha)
            alpha = best;
        if(best >= beta)
            break; //ab cutoff
    }
    return best;
}