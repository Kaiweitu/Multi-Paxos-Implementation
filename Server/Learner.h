#pragma once
#include "Header.h"

using namespace std;

class Learner {
    private:
        mutex innerMutex;
        int next_apply = 0;
        int quorum = 0;
        map<int, map<pair<int, int>, int>> cmd_map;
        map<int, struct sockaddr_in> client_addrs;
        map<pair<int, int>, command_map> command_map;
        
        vector<string> log;

        static const int ACCEPT_MESSAGE = 0;
        static const int HEARTBEAT_MESSAGE = 1;
        
        void handleAcceptMessage(acceptMsg &msg);
        void handleHbMessage(learnerHeartBeatMsg &msg);

    public:
        Learner();
        void start();

};