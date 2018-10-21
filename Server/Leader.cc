#pragma once
#include "Header.h"

using namespace std;

void Leader::start() {
    while (true) {
        string command = Server::leaderQue.pop();
        int seq, cid;
        string sentence;
        handleCommand(command, seq, cid, sentence);
        
        updateViewNumber();     
        findNextUnchosenLog() 
        while (prepare()) ;
        propose();
    }
}

/*
consider the following senerio:
For a replica, log entries:  C C C A C C C E 
                                view 2
Now, replica in View 10 and becomes a leader. He will propose value for the 4th entry and come across an error.
C <=> Chosen A <=> Accept E <=> Empty

conclusion: we need maintain the view number for each time slot (the view we accept that number).
*/

void Leader::handleCommand(const string& command, int& seq, int& cid, string& sentence) {
    istringstream parser(command);
    parser >> seq >> cid >> sentence;
}

void Leader::prepare() {

}

void Leader::propose() {

}

