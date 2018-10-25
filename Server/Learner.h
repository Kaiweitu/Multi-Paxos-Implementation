#pragma once
#include "Header.h"

using namespace std;

struct learner_data{
    int next_apply = 0;
    int quorum = 0;
    map<int, map<pair<int, int>, int>> cmd_map;
    map<int, struct sockaddr_in> client_addrs;
    map<pair<int, int>, string> command_map;
    vector<string> log;
};

class Learner {
    private:
        static mutex innerMutex;
        static const int ACCEPT_MESSAGE = 0;
        static const int HEARTBEAT_MESSAGE = 1;
        static const int SUCCESS_MESSAGE = 2;

        static void handleAcceptMessage(learner_data &data, acceptMsg &msg);
        static void handleHbMessage(learner_data &data, learnerHeartBeatMsg &msg);
        static void handleSuccessMessage(learner_data &data, successMsg &msg);
        static void applyMessage(learner_data &data);
        static void sendHbMessage();
        static bool checkChosen(learner_data &data, int slot);
        learner_data data;
        
    public:
        Learner();
        void start();

};