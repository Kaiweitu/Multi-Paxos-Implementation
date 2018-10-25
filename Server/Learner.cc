#pragma once
#include "Header.h"

using namespace std;


Learner::Learner() {
    this -> quorum = Server::addrs.size() / 2 + 1;
    _(dcout("[Learner] quorum is " + to_string(this -> quorum);)
}

void Learner::start() {
    while (1) {
        string message_str = Server::learnerQue.pop();
        _(dcout("[Leaner] Receive: " + msg);)
        stringstream ss(message_str);
        int type;
        ss >> type;
        int index = message_str.find_first_of(' ');
        string msg_str(message_str.begin() + index + 1, message_str.end());
        if (type == Learner::ACCEPT_MESSAGE) {
            acceptMsg msg = acceptMsg::deserialize(msg_str);
            thread t(this -> handleAcceptMessage(msg));
        } else if (type == Learner::HEARTBEAT_MESSAGE) {
            // TODO: Heartbeat Message;
        }

    }
}

void handleAcceptMessage(acceptMsg &msg) {
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(msg.port);
    client_addr.sin_addr.s_addr = msg.client_IP;

    unique_lock<mutex> lock(this -> innerMutex);
    if (this -> client_addrs.find(msg.client_ID) == this -> client_addrs.end()) {
        client_addrs[msg.client_ID] = client_addr;
    }
    pair<int, int> command_id = make_pair<msg.client_ID, msg.seq>;
    unique_lock<mutex> server_lock(Server::innerMutex);
    if (Server::logs[])
    server_lock.unlock();

    if (this -> cmd_map.find(msg.slot) != this -> cmd_map.end()) {
        auto it = this -> cmd_map[msg.slot].find(command_id);
        if (it != this -> cmd_map[msg.slot].end()) {
            it -> second += 1;
            if (it -> second >= this -> quorum) {
                string command = std::move(this -> command_map[command_id]);
                _(dcout("[Leaner] Slot " + to_string(msg.slot) + " chose value: " + it -> first -> second);)
                // write into the log
                log.push_back(it -> first -> second);
            }
        } else {
            this -> cmd_map[msg.slot][make_pair<msg.client_ID, msg.command>] = 1;
        }
    }


}