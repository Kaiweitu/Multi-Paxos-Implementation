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

long int getCurrentTime();

struct acceptMsg {
    int slot;
    int seq;
    int client_ID;
    int port;
    // Network Byte Orders
    unsigned long client_IP;
    string command;

    static string serialize(acceptedMsg &msg) {
        ostringstream oss;
        oss << slot << ' ' << client_IP << ' ' << port << ' ' << seq
        << client_ID << ':' << command;
        
        return oss.str();
    }

    static acceptMsg deserialize(string &msg) {
        acceptMsg msg;
        istringstream iss(msg);
        
        int index = msg.find_first_of(':');
        string cmd(msg.begin() + index + 1, msg.end());
        msg.command = std::move(cmd);
        iss >> msg.slot >> msg.client_IP >> msg.port >> msg.seq >> msg.client_ID;

        return std::move(msg);
    }
}

struct learnerHeartBeatMsg {
    int first_unchosen_index;
    int server_id;
}



