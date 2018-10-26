#pragma once
#include "Header.h"

using namespace std;

struct LeaderPrepareThreadData {
    string messageToPropose;
    int rejectNum;
    int maxView;
    int curViewNum;
    int slot;
    int finishNum;
    uint32_t userIP;
    int port;
    int seq;
    int CID;
    bool chosen;
    mutex innerMutex;
    condition_variable cv;
    LeaderPrepareThreadData(int unchosenSlot, int _curViewNum) : 
        messageToPropose("") ,rejectNum(0), maxView(-1), curViewNum(_curViewNum), slot(unchosenSlot), finishNum(0), chosen(false) 
        {}
};

struct LeaderPrepareData{
    struct sockaddr_in sockAddr;
    LeaderPrepareThreadData* threadData;
    LeaderPrepareData(struct sockaddr_in& _sockAddr, LeaderPrepareThreadData* _data) : sockAddr(_sockAddr), threadData(_data) {}
};

struct LeaderProposeThreadData {
    ProposeMsg& proposeMsg;
    int rejectNum;
    int finishNum;
    mutex innerMutex;
    condition_variable cv;
    LeaderProposeThreadData(ProposeMsg& _proposeMsg) : proposeMsg(_proposeMsg), rejectNum(0), finishNum(0) {}
};

struct LeaderProposeData {
    struct sockaddr_in sockAddr;
    LeaderProposeThreadData* threadData;
    LeaderProposeData(struct sockaddr_in& _sockAddr, LeaderProposeThreadData* _data) : sockAddr(_sockAddr), threadData(_data) {}
};

class Leader {
private:
    static void proposeHelper(LeaderProposeData* data);
    static void prepareHelper(LeaderPrepareData* data); 
    static void makePrepareMessage(LeaderPrepareData* data, string& msg);

    // TODO: make consistent
    static void processReplyMessage(const PrepareReply& preReply, LeaderPrepareData* data);
    static void processProposeReplyMsg(LeaderProposeData* data, ProposeReply& pReply);
    static int calculateViewNum();
    
    template <class MainClass, class ThreadClass>
    static void connectAllAcceptersAndSendMessage(void (*fun_ptr)(MainClass*), ThreadClass* threadData, unique_lock<mutex>& lck);
public:
    Leader(){
    };
    static void start();
    static void handleCommand(const string& command, int& seq, int& cid, string& sentence, uint32_t& userIP, int& port);
    static bool prepare(int unchosenSlot, int curViewNum);
    static void propose(ProposeMsg& proposeMsg);
};