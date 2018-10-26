#pragma once
#include "Header.h"

using namespace std;

class Acceptor {

public:
    static void processPrepareMsg(const string& msg, int FD);
    static void acceptTheSlot(const ProposeMsg& proposeMsg);
    static void sendAcceptMsgToLearner(acceptMsg& myAcceptMsg);
    static void processProposeMsg(const string& msg, int FD);
    static void replyProposeAcceptMsg(int fileDescriptor, ProposeMsg& prepareMsg);
    static void replyProposeRejectMsg(int fileDescriptor, ProposeMsg& prepareMsg);
    static void replyRejectMsg(int fileDescriptor, PrepareMsg& prepareMsg);
    static void replyFollowMsg(int fileDescriptor, PrepareMsg& prepareMsg);
    static void start();
};

