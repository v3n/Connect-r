//
//  main.m
//  Connect-r
//
//  Created by Jonathan Howard on 3/26/14.
//  Copyright (c) 2014 Jonathan Howard. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "GameState.h"
#include <iostream>
#include <dispatch/queue.h>
#include <queue>
#include "SearchHandler.h"


using namespace std;

int main(int argc, const char * argv[])
{

    @autoreleasepool {
        
        // insert code here...
        NSLog(@"Hello, World!");
        
        
    }
    
    Game *g = new Game();
    
    //I move first:
//    g->move(4);
    
    for ( int i = 0; ; i++ ) {
        cout << "GameState:" << endl;
        cout << g->to_string() << endl;
        
//        cout << "Please input a move: ";
//        string s;
//        getline(cin, s);
//        g->move(stoi(s));

//        if(g->checkForWin())
//            cout << "GAME WON!" << endl;
        
        move_t move = findNextMove(g);
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

