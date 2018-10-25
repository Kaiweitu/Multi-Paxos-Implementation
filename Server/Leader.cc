#pragma once
#include "Header.h"

using namespace std;

void Leader::start() {
    int curIndex = 0;
    while (true) {
        string command = Server::leaderQue.pop();
        
        try {
            int seq, cid;
            string sentence;
        
            int curViewNum = calculateViewNum();
            handleCommand(command, seq, cid, sentence);
        
            curIndex = findNextUnchosenLog(curIndex);    
            while (!prepare(curIndex, curViewNum)) 
                curIndex = findNextUnchosenLog(curIndex);;
            propose();
        }
        catch (...) { // receive reject
            leaderQue.makeEmpty();
        }
    }
}


int Leader::calculateViewNum() {
    Server::maxViewMutex.lock();
    Server::addrsMutex.lock();
    int baseNum = Server::maxViewNum / Server::addrs.size() * Server::addrs.size();
    Server::addrsMutex.unlock(); 
    Server::maxViewMutex.unlock();
    return baseNum + Server::sId;
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
/*
can such a situation exist?
A leader propose a different value on the slot that others has chosen a value.
1. receive reject -> directly abandon
2. 
*/

void Leader::prepareHelper(LeaderPrepareData* data) {
    string message = "";
    makePrepareMessage(data, message);
    sendAndRecvMessage(data->sockAddr, message);
    processPrepareReplyMessage(message, data);
    
    data->threadData->innerMutex.lock();
    




    if (data.finishNum == Server::addrs.size()) { // the last one
        data->threadData->innerMutex.unlock();
        delete data->threadData;
        delete data;
        return;
    }


    data->threadData->inner_mutex.unlock();
    delete data;
    
}

bool Leader::prepare(int unchosenSlot, int curViewNum) {
    // Presonal believe it is thread safe
    // 1 stands for the main thread
    LeaderPrepareThreadData* threadData = new LeaderPrepareThreadData();
    // send message to each server
    unique_lock<mutex> lck(threadData->innerMutex);

    Server::addrsMutex.lock();
    int majoritySize = Server::addrs.size() / 2 + 1;
    for (auto& addr : Server::addrs) {
        LeaderPrepareData* newData = new LeaderPrepareData(addr, threadData);
        thread t(prepareHelper, newData);
    }
    Server::addrsMutex.unlock();

    while (threadData->finishNum < majoritySize || !threadData->rejectNum )
        threadData->cv.wait(lck);
    
    // handle different situation
    // if receive reject
    if (threadData->rejectNum) {
        threadData->finishNum += 1;
        throw runtime_error("receive reject");    
    }
    
    
}


void Leader::propose() {

}

