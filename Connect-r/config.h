//
//  config.h
//  Connect-r
//
//  Created by Jonathan Howard on 3/31/14.
//  Copyright (c) 2014 Jonathan Howard. All rights reserved.
//

#ifndef Connect_r_HeuristicScores_h
#define Connect_r_HeuristicScores_h

//board dimensions
#define WIDTH 7
#define HEIGHT 6
#define C_R 4

//helper variables for board dimension
#define HEIGHTP1 (HEIGHT + 1)
#define HEIGHTP2 (HEIGHT + 2)
#define BOARDSIZE (WIDTH*HEIGHT)
#define BOARDSIZEP1 (WIDTH*HEIGHTP1)

//to_string characters
#define EMPTY_SPACE '-'
#define RED_SPACE 'O'
#define BLACK_SPACE 'X'

//millisecond duration to search
#define TIMEOUT 2000

//search ply
#define HORIZON 8

//heuristic scores
#define H_MAX 2147483640
#define WIN 512
#define DRAW 32
#define LOSS 1

#endif
