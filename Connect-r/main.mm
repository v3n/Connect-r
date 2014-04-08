//
//  main.m
//  Connect-r
//
//  Created by Jonathan Howard on 3/26/14.
//  Copyright (c) 2014 Jonathan Howard. All rights reserved.
//


#include "config.h"
#include "GameState.h"

#include <dispatch/queue.h>

#include <iostream>
#import <Foundation/Foundation.h>

using namespace std;

int main(int argc, const char * argv[])
{
    Game *g = new Game();
    
    for ( int i = 0; ; i++ ) {
        cout << "GameState:" << endl;
        cout << g->to_string() << endl;
        
//        cout << "Please input a move: ";
//        string s;
//        getline(cin, s);
//        g->move(stoi(s));

//        if(g->checkForWin())
//            cout << "GAME WON!" << endl;
        
        move_t move = g->findNextMove();
        cout << "Best Move: " << move << endl;
        g->move(move);
        
        cout << "GameState:" << endl;
        cout << g->to_string() << endl;
        
        if(g->checkForWin()) {
            cout << "GAME WON!" << endl;
            break;
        }
    }
    return 0;
}

