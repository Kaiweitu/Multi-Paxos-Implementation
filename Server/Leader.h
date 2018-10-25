#pragma once
#include "Header.h"

using namespace std;

struct LeaderPrepareThreadData {
    string messageToPropose;
    int rejectNum;
    int maxView;
    int curViewNum;
    int slot;
    mutex innerMutex;
    condition_variable cv;
    int finishNum;
    LeaderPrepareThreadData(int unchosenSlot, int _curViewNum) : 
        messageToPropose("") ,rejectNum(0), maxView(-1), curViewNum(_curViewNum), slot(unchosenSlot), finishNum(0) {}
};

struct LeaderPrepareData{
    struct sockaddr_in sockAddr;
    LeaderPrepareThreadData* threadData;
    LeaderPrepareData(struct sockaddr_in& _sockAddr, LeaderPrepareThreadData* _data) : sockAddr(_sockAddr), threadData(_data) {}
};

class Leader {
private:
    static void prepareHelper(LeaderPrepareData* data); 
    static void makePrepareMessage(LeaderPrepareData* data, string& msg);
    static void processReplyMessage(const PrepareReply& preReply, LeaderPrepareData* data);
    int calculateViewNum();
public:
    Leader(){
    };
    void start();
    void handleCommand(const string& command, int& seq, int& cid, string& sentence);
    bool prepare(int unchosenSlot, int curViewNum);
    void propose();
};