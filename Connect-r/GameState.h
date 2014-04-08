//
//  GameState.h
//  Connect-r
//
//  Created by Jonathan Howard on 3/28/14.
//  Copyright (c) 2014 Jonathan Howard. All rights reserved.
//

#ifndef __Connect_r__GameState__
#define __Connect_r__GameState__

#include "config.h"

#include <dispatch/dispatch.h>

#include <bitset>
#include <array>
#include <vector>
#include <iostream>


//Why are is this a C++ class instead of C or or an Objective-C class?
//The overhead of Objective-C classes & types made it unattractive. (smart
//pointers are nice for non-performance critical applications. I chose C++
//over C as C++ classes as lets us thread state tree analysis nicely.

//A couple optimizations we're using here:
// i % 2 -> i & 1
// ((i + 1) % 2 -> (i & 1) ^ 1
// It's unsure if these optimizations provide a significant performance
// increase over such a short time, but I'm putting them there in the hope
// that they do. Even saving two CPU instructions should provide a increase
// over two seconds when the function is repeatedly called.

//Due to a troubling memory bug with Boost on OS X, we're using
//std::bitset instead of boost::dynamic_bitset. This requires the
//use of preprocessor defines for fixed size bitsets.


typedef int move_t;
typedef std::bitset<BOARDSIZEP1> bitboard;

class Game {
    std::array<bitboard *, 2> boards;
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    
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

public:
    enum Color {
        Red,
        Black
    };
    
    int Moves[BOARDSIZE];  //move transition (memory efficient instead of
                                    //generating a new class for each boardstate)
    int nMoves = 0; // number of moves; normally referred to as 'ply'
                    //(which I found out halfway through coding)
    int Columns[WIDTH]; // number of pieces in each column
    
    Game() {
        boards[Black] = new bitboard(0);
        boards[Red] = new bitboard(0);
        
        for (int i = 0; i < WIDTH; i++)
            Columns[i] = 0;
        for(int i = 0; i < BOARDSIZE; i++) {
            Moves[i] = -1;
        }
    }
    
    Game(const Game& g) {
        memcpy(&boards, &g.boards, 2*sizeof(bitboard));
        memcpy(&Moves, &g.Moves, BOARDSIZE*sizeof(int));
        memcpy(&Columns, &g.Columns, WIDTH*sizeof(int));
        nMoves = g.nMoves;
    }
    
    Game& operator=(const Game& g) {
        if (this == &g)
            return *this;
        
        memcpy(&boards, &g.boards, 2*sizeof(bitboard));
        memcpy(&Moves, &g.Moves, BOARDSIZE*sizeof(int));
        memcpy(&Columns, &g.Columns, WIDTH*sizeof(int));
        nMoves = g.nMoves;
        
        return *this;
    }
    
    ~Game() {
        return;
    }
    
    int isMoveLegal(move_t move) {
        //combine the two boards, retrieve the bit and reverse it.
        return Columns[move] < HEIGHT;
    }
    
    // FIXME: This currently works for Connect-4, but not for all r
    bool checkForWin() {
        bitboard board = *boards[(nMoves - 1) & 1];
        
        bitboard horizontal = board & (board >> HEIGHTP1);
        bitboard vertical = board & (board >> 1);
        bitboard leftright = board & (board >> HEIGHTP2); //handle diagonal
        bitboard rightleft = board & (board >> HEIGHT);
        
        return (vertical & (vertical >> 2)).any() |
        (horizontal & (horizontal >> 2*HEIGHTP1)).any() |
        (leftright & (leftright >> 2*HEIGHTP2)).any() |
        (rightleft & (rightleft >> 2*HEIGHT)).any();
        
    }
    std::string to_string() {
        std::string r;

        //transposition col by row -> row by col
        //note this results in (0,0) begin in the top right
        for(int i = 0; i < WIDTH - 1; i++) { //x
            r += '\n';
            for(int j = HEIGHTP1 - 1; j >= 0; j--) { //y
                int index = (j*HEIGHTP1) + i;
                
                if((* boards[Black])[index] == 0 && (* boards[Red])[index] == 0) {
                    r += EMPTY_SPACE;
                } else if ((* boards[Black])[index] != 0) {
                    r += BLACK_SPACE;
                } else if ((* boards[Red])[index] != 0) {
                    r += RED_SPACE;
                }
            }
        }
        
        //reverse string to place origin at bottom left
        return std::string(r.rbegin(), r.rend());
    }
    
    bool move(move_t move) {
        if(isMoveLegal(move)) {
            (* boards[nMoves & 1])[(move * HEIGHTP1) + Columns[move]] = 1;
            Columns[move] += 1;
            Moves[nMoves++] = move;
            return true;
        }
        return false;
    }
    
    void unmove() {
        nMoves--;
        (* boards[nMoves & 1])[(Moves[nMoves] * HEIGHTP1) + --Columns[Moves[nMoves]]] = 0;
    }
    
    std::vector<move_t> getOrderedMoves() {
        //Prioritize moves by number of tiles already present in that column
        std::vector<move_t> r = std::vector<move_t>();
        for (int i = 0; i < WIDTH; i++) {
            if(Columns[i] < HEIGHT)
                r.push_back(i);
        }
        sort(r.begin(), r.end(),
             [=](const move_t a, const move_t b)
             {
                 return Columns[a] > Columns[b];
             });
        return r;
    }
    
    int rate() { //heuristic function
        if (checkForWin())
            return WIN + (5*(BOARDSIZE - nMoves)); //offset
        
        int score = 0;
        
        bitboard board = (* boards[nMoves & 1]);
        bitboard otherBoard = (* boards[(nMoves & 1) ^ 1]);
        
        //Checking vertical
        bitboard vertical(board);
        for (int i = 1; i < C_R; i++) {
            vertical = board & (board >> i);
            score += vertical.count() * i;
        }
        //enemy moves
        vertical = bitboard(otherBoard);
        for (int i = 1; i < C_R; i++) {
            vertical = board & (otherBoard >> i);
            score -= vertical.count() * i + 1;
        }
        
        //We're going to rate diagonal's a little heavier
        //because they're more computationally complex
        //hopefully resulting in a harder search for the
        //opponent
        bitboard diagonal(board);
        for (int i = 1; i < C_R; i++) { //left to right
            diagonal = board & (board >> i*HEIGHTP2);
            score += (diagonal.count() * i) - 1;
        }
        diagonal = bitboard(board);
        for (int i = 1; i < C_R; i++) { //right to left
            diagonal = board & (board >> i*HEIGHT);
            score += (diagonal.count() * i) - 1;
        }
        //enemy moves
        diagonal = bitboard(otherBoard);
        for (int i = 1; i < C_R; i++) { //left to right
            diagonal = board & (board >> i*HEIGHTP2);
            score -= (diagonal.count() * i);
        }
        diagonal = bitboard(otherBoard);
        for (int i = 1; i < C_R; i++) { //right to left
            diagonal = board & (board >> i*HEIGHT);
            score -= (diagonal.count() * i);
        }

        //horizontal
        bitboard horizontal(board);
        for (int i = 1; i < C_R; i++) {
            horizontal = horizontal & (board >> i*HEIGHTP1);
            score += horizontal.count() * i;
        }
        //enemy moves
        horizontal = bitboard(board);
        for (int i = 1; i < C_R; i++) {
            horizontal = horizontal & (board >> i*HEIGHTP1);
            score -= horizontal.count() * i + 1;
        }
        
        return score;
    }
    
    move_t findNextMove() {
        __block struct {
            __block int score[WIDTH];
        } scores;
        
        for(int i = 0; i < WIDTH; i++)
            scores.score[i] = -INT_MIN;
        
        dispatch_group_t group = dispatch_group_create();
        Game * currentGame;
        
        std::vector<move_t> moves = this->getOrderedMoves();
        
        for(std::vector<move_t>::iterator i = moves.begin(); i != moves.end(); ++i) {
            if(this->move(*i)) {
                currentGame = new Game((const Game)*this);
                this->unmove();
                
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
    
};


#endif /* defined(__Connect_r__GameState__) */
