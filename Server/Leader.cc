#pragma once
#include "Header.h"

using namespace std;

void Leader::start() {
    int curIndex = 0;
    while (true) {
        string command = Server::leaderQue.pop();
        int seq, cid;
        string sentence;

        handleCommand(command, seq, cid, sentence);
        
        curIndex = findNextUnchosenLog(curIndex);
        while (!prepare(curIndex)) 
            curIndex = findNextUnchosenLog(curIndex);;
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

int Leader::findNextUnchosenLog(int curIndex) {
    unique_lock<mutex> iMutex(Server::innerMutex);
    for (size_t i = curIndex; i < Server::logs.size(); ++ i)
        if (!Server::logs[i].choosen) return i;

    Server::logs.push_back(LogEntry());
    return Server::logs.size() - 1;
}

void Leader::handleCommand(const string& command, int& seq, int& cid, string& sentence) {
    istringstream parser(command);
    parser >> seq >> cid >> sentence;
}

bool Leader::prepare(int unchosenSlot) {
    return true;
}

void Leader::propose() {

}

