#pragma once
#include "Header.h"

using namespace std;

class Acceptor {

public:
    void processPrepareMsg(const string& msg, int FD);
    void acceptTheSlot(const ProposeMsg& proposeMsg);
    void sendAcceptMsgToLearner(acceptMsg& myAcceptMsg);
    void processProposeMsg(const string& msg, int FD);
    void replyProposeAcceptMsg(int fileDescriptor, ProposeMsg& prepareMsg);
    void replyProposeRejectMsg(int fileDescriptor, ProposeMsg& prepareMsg);
    void replyRejectMsg(int fileDescriptor, PrepareMsg& prepareMsg);
    void replyFollowMsg(int fileDescriptor, PrepareMsg& prepareMsg);
    void start();
};

