#pragma once
#include "Header.h"

using namespace std;

struct learner_data{
    mutex innerMutex;
    int next_apply = 0;
    int quorum = 0;
    map<int, map<pair<int, int>, int>> cmd_map;
    map<int, struct sockaddr_in> client_addrs;
    map<pair<int, int>, string> command_map;
    vector<string> log;
};

class Learner {
    private:
        static learner_data data;

        static const int ACCEPT_MESSAGE = 0;
        static const int HEARTBEAT_MESSAGE = 1;
        
        
    public:
        Learner();
        void start();
        static void handleAcceptMessage(learner_data &data, acceptMsg &msg);
        static void handleHbMessage(learnerHeartBeatMsg &msg);


};