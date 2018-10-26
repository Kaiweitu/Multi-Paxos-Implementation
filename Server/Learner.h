#pragma once
#include "Header.h"

using namespace std;

struct slotEntry {
    int client_ID;
    int seq;
    int view_num;
    string command;

    slotEntry() : client_ID(-1), seq(-1), view_num(-1){}
    slotEntry(int _client_ID, int _seq, int _view_num) : client_ID(_client_ID), seq(_seq), view_num(_view_num){}
};

struct learner_data{
    int next_apply = 0;
    int quorum = 0;
    // Has problem here
    vector<vector<slotEntry>> acceptor_vec;
    // 
    map<int, struct sockaddr_in> client_addrs;
    // map<pair<int, int>, string> command_map;
    vector<string> log;
};

class Learner {
    private:
        static mutex innerMutex;

        static void handleAcceptMessage(learner_data &data, acceptMsg msg);
        static void handleHbMessage(learner_data &data, learnerHeartBeatMsg msg);
        static void handleSuccessMessage(learner_data &data, successMsg msg);
        static void applyMessage(learner_data &data);
        static void sendHbMessage();
        static bool checkChosen(int slot);
        static learner_data data;
        
    public:
        Learner();
        static void start();

};