#pragma once
#include "Header.h"
using namespace std;









































































































































// receiver type
const int LEADER = 0;
const int ACCEPTOR = 1;
const int LEARNER = 2;
const int CLIENT_REQUEST = 3;

// message type
const int MESSAGE_PREPARE = 0;
const int MESSAGE_PREPOSE = 1;

struct PrepareMsg {
    int view;
    int slot;
    
    void serialize (string& msg) {
        msg = to_string(ACCEPTOR) + " " + to_string(view) + " " + to_string(slot);
    };

    void deserialize(const string& msg) {
        istringstream parser(msg);
        parser >> view >> slot;
    } 
};

struct PrepareReply {
    int view;
    int slot;
    char AorR;
    char noMore;
    int oldView;
    string oldCommand;

    void serialize (string& msg) {
        msg = to_string(view) + " " + to_string(slot) + " " + AorR + " " + noMore + " " + to_string(oldView) + ":" + oldCommand;
    };

    void deserialize(const string& msg) {
        istringstream parser(msg);
        parser >> view >> slot >> AorR >> noMore >> oldView;
        oldCommand = msg.substr(msg.find(':') + 1, msg.size() - msg.find(':') - 1);
    }

};

struct ProposeMsg {
    int view;
    int slot;
    unsigned long userIP;
    int port;
    int seq;
    int CID;
    string command;

    void serialize (string& msg) {
        msg = to_string(ACCEPTOR) + " " + to_string(view) + " " + to_string(slot) + " " + to_string(userIP) + " " + to_string(port) 
            + " " + to_string(seq) + " " + to_string(CID) + ";" + command;
    };

    void deserialize(const string& msg) {
        istringstream parser(msg);
        parser >> view >> slot >> userIP >> port >> seq >> CID;
        command = msg.substr(msg.find(';') + 1, msg.size() - msg.find(';') - 1);
    } 
};

struct ProposeReply {
    int view;
    int slot;
    char AorR;

    void serialize (string& msg) {
        msg = to_string(view) + " " + to_string(slot) + " " + AorR;
    };

    void deserialize(const string& msg) {
        istringstream parser(msg);
        parser >> view >> slot >> AorR;
    } 
};

void dCout(const string& msg);

void sendAndRecvMessage(struct sockaddr_in& addr, string& msg);

