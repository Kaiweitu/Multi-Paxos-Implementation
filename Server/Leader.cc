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
        
            curIndex = Server::findNextUnchosenLog(curIndex);    
            while (!prepare(curIndex, curViewNum)) 
                curIndex = Server::findNextUnchosenLog(curIndex);;
            propose();
        }
        catch (...) { // receive reject
            Server::leaderQue.makeEmpty();
        }
    }
}


int Leader::calculateViewNum() {
    Server::maxViewMutex.lock();
    int baseNum = Server::maxViewNum / Server::addrs.size() * Server::addrs.size();
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

void Leader::makePrepareMessage(LeaderPrepareData* data, string& msg) {
    PrepareMsg myMsg;
    myMsg.view = data->threadData->curViewNum;
    myMsg.slot = data->threadData->slot;
    myMsg.serialize(msg);
}

void Leader::processReplyMessage(const PrepareReply& preReply, LeaderPrepareData* data) {
    if (preReply.AorR == 'R') {
        data->threadData->rejectNum += 1;
        data->threadData->cv.notify_one();
        return;
    } 

    if (preReply.AorR == 'C' || preReply.AorR == 'A') {
        // the acceptor slot is not empty;
        if (data->threadData->maxView < preReply.oldView) { 
            data->threadData->maxView = preReply.oldView;
            data->threadData->messageToPropose = preReply.oldCommand;
        }
    }

    if (data->threadData->finishNum >= Server::addrs.size() / 2) {
        data->threadData->cv.notify_one();
    }

}

void Leader::prepareHelper(LeaderPrepareData* data) {
    string message = "";
    makePrepareMessage(data, message);
    sendAndRecvMessage(data->sockAddr, message);
    
    PrepareReply preReply;
    preReply.deserialize(message);

    data->threadData->innerMutex.lock();
    processReplyMessage(preReply, data);
    
    if (data->threadData->finishNum == Server::addrs.size()) { // the last one
        data->threadData->innerMutex.unlock();
        delete data->threadData;
        delete data;
        return;
    }

    data->threadData->finishNum += 1;
    data->threadData->innerMutex.unlock();
    delete data;
    
}

bool Leader::prepare(int unchosenSlot, int curViewNum) {
    // Presonal believe it is thread safe
    // 1 stands for the main thread
    LeaderPrepareThreadData* threadData = new LeaderPrepareThreadData(unchosenSlot, curViewNum);
    // send message to each server
    unique_lock<mutex> lck(threadData->innerMutex);

    int majoritySize = Server::addrs.size() / 2 + 1;
    for (auto& addr : Server::addrs) {
        LeaderPrepareData* newData = new LeaderPrepareData(addr, threadData);
        thread t(prepareHelper, newData);
    }

    while (threadData->finishNum < majoritySize || !threadData->rejectNum )
        threadData->cv.wait(lck);
    
    
    // handle different situation
    // if receive reject
    threadData->finishNum += 1;
    if (threadData->rejectNum) {
        if (threadData->finishNum == Server::addrs.size() + 1) { // the last one
            threadData->innerMutex.unlock();
            delete threadData;
        }
        throw runtime_error("receive reject");    
    }

    // if not
    bool ret = false;
    if (threadData->maxView != -1)  // need to propose the old value
        propose();
    else  // can propose new value
        ret = true;
    


    if (threadData->finishNum == Server::addrs.size() + 1) { // the last one
        threadData->innerMutex.unlock();
        delete threadData;
    }
    else 
        threadData->innerMutex.unlock();
        
    return true;
    
}


void Leader::propose() {

}

