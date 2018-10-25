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

void Acceptor::ProcessPrepareMsg() {
    int reply = 0;
    char buffer[MAXBUFFERSIZE];
    
    recv(sock, &replySize, sizeof(int), MSG_WAITALL); replySize = ntohl(replySize);
    recv(sock, buffer, replySize, MSG_WAITALL);
    string msg(buffer);

    
}


void Acceptor::start() {
    while (true) {
        string message = Server::acceptorQue.pop();
        istringstream parser(message);
        int msgType; parser >> msgType;
        message = message.substr(0, message.find(' '));
        if (msgType == MESSAGE_PREPARE) {
            
        }
        else if (msgType == MESSAGE_PREPOSE) {

        }

    }
}

