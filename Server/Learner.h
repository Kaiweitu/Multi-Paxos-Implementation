#pragma once
#include "Header.h"

using namespace std;

struct slotEntry {
    int client_ID;
    int seq;
    int accept_seq;
    string command;

    slotEntry() : client_ID(-1), seq(-1), accept_seq(-1){}
    slotEntry(int _client_ID, int _seq, int _accept_seq) : client_ID(_client_ID), seq(_seq), accept_seq(_accept_seq){}
};

struct learner_data{
    int next_apply = 0;
    int quorum = 0;
    // Has problem here
    vector<vector<slotEntry>> acceptor_vec;
    // 
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
        static bool checkChosen(int slot);
        learner_data data;
        
    public:
        Learner();
        void start();

};