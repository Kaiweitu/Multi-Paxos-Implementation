#pragma once
#include "Header.h"

using namespace std;


// Todo: one return no more accepted, no need to prepare for that value

void Leader::start() {
    int curIndex = 0;
    while (true) {
        string command = Server::leaderQue.pop();
        
        try {
            dCout("Leader : Get Request from Client");
            dCout(command);
            string sentence;
            uint32_t userIP;
            int port;
            int seq, cid;
            int curViewNum = calculateViewNum();
            
            handleCommand(command, seq, cid, sentence, userIP, port);
            dCout("Leader: Sentence is");
            dCout(sentence);

            curIndex = Server::findNextUnchosenLog(curIndex);    
            while (curIndex == Server::skipedSlot || !prepare(curIndex, curViewNum)) 
                curIndex = Server::findNextUnchosenLog(curIndex);
            
            dCout("Leader: Start to propose msg");
            ProposeMsg proposeMsg(curViewNum, curIndex, userIP, port, seq, cid, sentence);
            
            _(
                string temp;
                proposeMsg.serialize(temp);
                dCout(temp);   
            )

            propose(proposeMsg);
        }
        catch (...) { // receive reject
            dCout("Leader: I'm not a leader any more.");
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


void Leader::handleCommand(const string& command, int& seq, int& cid, string& sentence, uint32_t& userIP, int& port) {
    istringstream parser(command);
    parser >> seq >> cid;
    sentence = command.substr(command.rfind(':') + 1, command.size() - command.rfind(':') - 1);
    istringstream anotherParser(sentence);
    anotherParser >> userIP >> port;
    sentence = command.substr(command.find(':') + 1, command.rfind(':') - 1 - command.find(':'));
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

    if (preReply.AorR == 'A') {
        // the acceptor slot is not empty;
        if (data->threadData->maxView < preReply.oldView) { 
            data->threadData->maxView = preReply.oldView;
            data->threadData->messageToPropose = preReply.oldCommand;
            data->threadData->userIP = preReply.userIP;
            data->threadData->port = preReply.port;
            data->threadData->seq = preReply.seq;
            data->threadData->CID = preReply.CID;
        }
    }
    else if (preReply.AorR == 'C') 
        data->threadData->chosen = true;
    

    
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

template <class MainClass, class ThreadClass>
void Leader::connectAllAcceptersAndSendMessage(void (*fun_ptr)(MainClass*), ThreadClass* threadData, unique_lock<mutex>& lck) {
    int majoritySize = Server::addrs.size() / 2 + 1;
    
    
    _(dCout( "finishNum:" + to_string(threadData->finishNum) );)
    _(dCout( "rejectNum:" + to_string(threadData->rejectNum) );)
    _(dCout( "majoritySize:" + to_string(majoritySize) );)


    for (auto& addr : Server::addrs) {
        _(dCout("Leader: create thread " + to_string(Server::addrs.size()) );)
        MainClass* newData = new MainClass(addr, threadData);
        thread(fun_ptr, newData).detach();
    }

    
    while (threadData->finishNum < majoritySize && !threadData->rejectNum) {
        
        if (threadData->cv.wait_for(lck, chrono::seconds(TIMEOUTTIME)) == cv_status::timeout) {
            if (threadData->finishNum < majoritySize && !threadData->rejectNum) {
                threadData->finishNum += 1;
                _(dCout("Leader: time out!"));
                return;
            }
        }
    }

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

}

bool Leader::prepare(int unchosenSlot, int curViewNum) {
    // 1 stands for the main thread
    LeaderPrepareThreadData* threadData = new LeaderPrepareThreadData(unchosenSlot, curViewNum);
    // send message to each server
    unique_lock<mutex> lck(threadData->innerMutex);
    
    _(dCout("Leader: unchosenSlot is " + to_string(unchosenSlot));)

    connectAllAcceptersAndSendMessage<LeaderPrepareData, LeaderPrepareThreadData> (Leader::prepareHelper, threadData, lck);
    
    _(dCout("Leader: collect majority results ");)
    // if not
    bool ret = false;
    if (threadData->chosen) ;
    else if (threadData->maxView != -1) {  // need to propose the old value
        ProposeMsg proposeMsg(curViewNum, unchosenSlot, threadData->userIP, threadData->port, 
            threadData->seq, threadData->CID, threadData->messageToPropose);
        propose(proposeMsg);
    }
    else  // can propose new value
        ret = true;
    


    if (threadData->finishNum == Server::addrs.size() + 1) { // the last one
        threadData->innerMutex.unlock();
        delete threadData;
    }
    else 
        threadData->innerMutex.unlock();
        
    return ret;
    
}

void Leader::processProposeReplyMsg(LeaderProposeData* data, ProposeReply& pReply) {
    if (pReply.AorR == 'R') {
        data->threadData->rejectNum += 1;
        data->threadData->cv.notify_one();
        return;
    } 
    
    if (data->threadData->finishNum >= Server::addrs.size() / 2) {
        data->threadData->cv.notify_one();
    }
}

void Leader::proposeHelper(LeaderProposeData* data) {
    string msg;
    data->threadData->proposeMsg.serialize(msg);
    sendAndRecvMessage(data->sockAddr, msg);
    
    ProposeReply pReply; 
    pReply.deserialize(msg);

    data->threadData->innerMutex.lock();
    processProposeReplyMsg(data, pReply);
    
    if (data->threadData->finishNum == Server::addrs.size()) {
        data->threadData->innerMutex.unlock();
        delete data->threadData;
        delete data;
        return;
    }

    data->threadData->innerMutex.unlock();
}

void Leader::propose(ProposeMsg& proposeMsg) {
    // 1 stands for the main thread
    LeaderProposeThreadData* threadData = new LeaderProposeThreadData(proposeMsg);
    // send message to each server
    unique_lock<mutex> lck(threadData->innerMutex);

    connectAllAcceptersAndSendMessage<LeaderProposeData, LeaderProposeThreadData> (Leader::proposeHelper, threadData, lck);

    if (threadData->finishNum == Server::addrs.size() + 1) { // the last one
        threadData->innerMutex.unlock();
        delete threadData;
    }
    else 
        threadData->innerMutex.unlock();
    
}

