#pragma once
#include "Header.h"
using namespace std;



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



