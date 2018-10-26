#pragma once
#include "Header.h"

using namespace std;
/*
TCP protocol
*/
/*
Acceptors.
when receiving the prepare message,
check which types of data has been received
    If receiving the prepared data,
        go to slot, check the view number;
        1. If original view || locked view number > view number;
            return reject.
        2. If the slot has been chosen,
            return chosen.
        3. If the slot has been accepted before,
            return accepted, the original view number, message and update the locked view number.
        4. Empty,
            return non-accepted.
*/
/*
when receiving the propose message,
check which types of data bas been received
    If receiving the propose data, 
        go to slot, check the view number;
        1. If original view || locked view number > view number;
            return reject.
        2. If the slot has been chosen and the value is different from the propose value,
            assert and run log.
        3. Otherwise,
            update the view number
            3.1 If the slot has been accepted and the value is different || not been accepted,
                accept the new value
            3.2 If the slot has been accepted and the value is same.
                do nothing.
*/

void Acceptor::processPrepareMsg(const string& msg, int FD) {
    PrepareMsg prepareMsg; prepareMsg.deserialize(msg);
    if (Server::maxViewNum > prepareMsg.view) 
        replyRejectMsg(FD, prepareMsg);
    else { 
        Server::maxViewMutex.lock();
        Server::maxViewNum = prepareMsg.view;
        Server::maxViewMutex.unlock();
        replyFollowMsg(FD, prepareMsg);
    }
}

void Acceptor::acceptTheSlot(const ProposeMsg& proposeMsg) {
    Server::innerMutex.lock();
    
    while (Server::logs.size() <= proposeMsg.slot)
        Server::logs.emplace_back(LogEntry());

    assert( !Server::logs[proposeMsg.slot].chosen || 
        (Server::logs[proposeMsg.slot].chosen && Server::logs[proposeMsg.slot].data == proposeMsg.command));
    
    Server::logs[proposeMsg.slot].accepted = true;
    Server::logs[proposeMsg.slot].data = proposeMsg.command;
    Server::logs[proposeMsg.slot].viewNum = proposeMsg.view;
    Server::logs[proposeMsg.slot].seq = proposeMsg.seq;
    Server::logs[proposeMsg.slot].client_ID = proposeMsg.CID;
    Server::logs[proposeMsg.slot].userIP = proposeMsg.userIP;
    Server::logs[proposeMsg.slot].port = proposeMsg.port;

    Server::innerMutex.unlock();

    acceptMsg myAcceptMsg;
    myAcceptMsg.slot = proposeMsg.slot;
    myAcceptMsg.server_id = Server::sId;
    myAcceptMsg.seq = proposeMsg.seq;
    myAcceptMsg.client_ID = proposeMsg.CID;
    myAcceptMsg.port = proposeMsg.port;
    myAcceptMsg.view_num = proposeMsg.view;
    myAcceptMsg.client_IP = proposeMsg.userIP;
    myAcceptMsg.command = proposeMsg.command;

    sendAcceptMsgToLearner(myAcceptMsg);
}

void Acceptor::sendAcceptMsgToLearner(acceptMsg& myAcceptMsg) {
     string msg = acceptMsg::serialize(myAcceptMsg);
     for (auto& addr : Server::addrs) 
         close(sendMessage(addr, msg));
}

void Acceptor::processProposeMsg(const string& msg, int FD) {
    ProposeMsg proposeMsg; proposeMsg.deserialize(msg);
    if (Server::maxViewNum > proposeMsg.view) 
        replyProposeRejectMsg(FD, proposeMsg);
    else {
        Server::maxViewMutex.lock();
        Server::maxViewNum = proposeMsg.view;
        Server::maxViewMutex.unlock();
        acceptTheSlot(proposeMsg);
        replyProposeAcceptMsg(FD, proposeMsg);
    }
}

void Acceptor::replyProposeAcceptMsg(int fileDescriptor, ProposeMsg& proposeMsg) {
    ProposeReply proposeReply;
    proposeReply.AorR = 'A';
    string msg; proposeReply.serialize(msg); 
    sendMessageHelper(fileDescriptor, msg);
    close(fileDescriptor);
}

void Acceptor::replyProposeRejectMsg(int fileDescriptor, ProposeMsg& proposeMsg) {
    ProposeReply proposeReply;
    proposeReply.AorR = 'R';
    string msg; proposeReply.serialize(msg); 
    sendMessageHelper(fileDescriptor, msg);
    close(fileDescriptor);
}

void Acceptor::replyRejectMsg(int fileDescriptor, PrepareMsg& prepareMsg) {
    PrepareReply prepareReply;
    prepareReply.AorR = 'R';
    string msg; prepareReply.serialize(msg); 
    sendMessageHelper(fileDescriptor, msg);
    close(fileDescriptor);
}

void Acceptor::replyFollowMsg(int fileDescriptor, PrepareMsg& prepareMsg) {
    PrepareReply prepareReply;
    Server::innerMutex.lock();
    
    if (Server::logs[prepareMsg.slot].chosen) {
        Server::innerMutex.unlock();
        prepareReply.AorR = 'C';
        string msg; prepareReply.serialize(msg); 
        sendMessageHelper(fileDescriptor, msg);
    }
    else if (Server::logs[prepareMsg.slot].accepted){
        prepareReply.oldCommand = Server::logs[prepareMsg.slot].data;
        prepareReply.oldView = Server::logs[prepareMsg.slot].viewNum;
        prepareReply.seq = Server::logs[prepareMsg.slot].seq;
        prepareReply.port = Server::logs[prepareMsg.slot].port;
        prepareReply.CID = Server::logs[prepareMsg.slot].client_ID;
        prepareReply.userIP = Server::logs[prepareMsg.slot].userIP;
        Server::innerMutex.unlock();

        prepareReply.AorR = 'A';
        prepareReply.view = prepareMsg.view;
        prepareReply.slot = prepareMsg.slot;
        
        string msg; prepareReply.serialize(msg); 
        sendMessageHelper(fileDescriptor, msg);
    }
    else {
        Server::innerMutex.unlock();
        prepareReply.AorR = 'A';
        prepareReply.oldView = -1;

        string msg; prepareReply.serialize(msg);
        sendMessageHelper(fileDescriptor, msg); 
    }

    close(fileDescriptor);
}


void Acceptor::start() {
    while (true) {
        string message = Server::acceptorQue.pop();
        istringstream parser(message);
        int msgType; parser >> msgType;
        message = message.substr(0, message.find(' '));
        int FD = atoi(message.substr(message.rfind(';') + 1, message.size() - message.rfind(';') - 1).c_str());
        message = message.substr(0, message.rfind(';'));
        if (msgType == MESSAGE_PREPARE) {
            processPrepareMsg(message, FD);
        }
        else if (msgType == MESSAGE_PREPOSE) {
            processProposeMsg(message, FD);
        }
    }
}

