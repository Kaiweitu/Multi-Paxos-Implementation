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

const int ACCEPT_MESSAGE = 0;
const int HEARTBEAT_MESSAGE = 1;
const int SUCCESS_MESSAGE = 2;

static int LOSS_RATE = 0;

struct PrepareMsg {
    int view;
    int slot;
    
    void serialize (string& msg) {
        msg = to_string(ACCEPTOR) + " " + to_string(MESSAGE_PREPARE) + " " + to_string(view) + " " + to_string(slot);
    };

    void deserialize(const string& msg) {
        istringstream parser(msg);
        parser >> view >> slot;
    } 
};

struct PrepareReply {
    int view = -1;
    int slot = -1;
    char AorR;
    char noMore;
    uint32_t userIP = 0;
    int port = -1;
    int seq = -1;
    int CID = -1;
    int oldView = -1;
    
    string oldCommand;

    void serialize (string& msg) {
        msg = to_string(view)  + " " + to_string(slot) + " " + AorR + " " + noMore + " " 
            + to_string(userIP) + " " + to_string(port) + " " + to_string(seq) + " " + to_string(CID) + " " 
            + to_string(oldView) + ";" + oldCommand;
    };

    void deserialize(const string& msg) {
        istringstream parser(msg);
        parser >> view >> slot >> AorR >> noMore >> userIP >> port >> seq >> CID >> oldView;
        oldCommand = msg.substr(msg.find(';') + 1, msg.size() - msg.find(';') - 1);
    };

};

struct ProposeMsg {
    int view;
    int slot;
    uint32_t userIP;
    int port;
    int seq;
    int CID;
    string command;
    ProposeMsg () {

    };
    ProposeMsg (int _view, int _slot, uint32_t _userIP, int _port, int _seq, int _CID, const string& _command) :
        view(_view), slot(_slot), userIP(_userIP), port(_port), seq(_seq), CID(_CID), command(_command) {

    };

    void serialize (string& msg) {
        msg = to_string(ACCEPTOR) + " " + to_string(MESSAGE_PREPOSE) + " " + to_string(view) + " " + to_string(slot) 
            + " " + to_string(userIP) + " " + to_string(port) + " " + to_string(seq) + " " + to_string(CID) + ";" + command;
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

int sendMessage(struct sockaddr_in &addr, string &msg);
void sendAndRecvMessage(struct sockaddr_in& addr, string& msg);

long int getCurrentTime();

struct acceptMsg {
    int slot;
    // Server seq_ID
    int server_id;
    // client seq_ID
    int seq;
    int client_ID;
    int port;
    int view_num;
    // Network Byte Orders
    uint32_t client_IP;
    string command;

    static string serialize(acceptMsg &msg) {
        ostringstream oss;
        oss << LEARNER << ' ' <<  ACCEPT_MESSAGE  <<  ' ' << msg.slot << ' ' << msg.server_id  << ' ' << msg.view_num 
        << ' ' << msg.client_IP << ' ' << msg.port << ' ' << msg.seq
        << ' ' << msg.client_ID << ':' << msg.command;
        
        return oss.str();
    }

    static acceptMsg deserialize(string &msg_buf) {
        acceptMsg msg;
        istringstream iss(msg_buf);
        
        int index = msg_buf.find_first_of(':');
        string cmd(msg_buf.begin() + index + 1, msg_buf.end());

     
        iss >> msg.slot >> msg.server_id >> msg.view_num >> msg.client_IP >> msg.port >> msg.seq >> msg.client_ID;
        msg.command = cmd;
        return std::move(msg);
    }
};

struct learnerHeartBeatMsg {
    int first_unchosen_index;
    int server_id;

    static string serialize(learnerHeartBeatMsg &msg) {
        ostringstream oss;
        oss << msg.first_unchosen_index << ' ' << msg.server_id;
        return oss.str();
    }

    static learnerHeartBeatMsg deserialize(string &msg_buf) {
        learnerHeartBeatMsg msg;
        istringstream iss(msg_buf);

        iss >> msg.first_unchosen_index >> msg.server_id;

        return std::move(msg);
    }
};

struct successMsg {
    int slot;
    int seq;
    int client_ID;
    int port;
    // Network Byte Orders
    uint32_t client_IP;
    string command;

    static string serialize(successMsg &msg) {
        ostringstream oss;
        oss << msg.slot << ' ' << msg.seq << ' ' << msg.client_ID << ' ' << msg.port << 
            ' ' << msg.client_IP << ':' << msg.command;
        return oss.str();
    }

    static successMsg deserialize(string &msg_buf) {
        successMsg msg;
        istringstream iss(msg_buf);
        
        int index = msg_buf.find_first_of(':');
        string cmd(msg_buf.begin() + index + 1, msg_buf.end());
        msg.command = std::move(cmd);

        iss >> msg.slot >> msg.seq >> msg.client_ID >> msg.port >> msg.client_IP;
        return std::move(msg);
    }
};

void sendMessageHelper(int sock, const string& msg);



