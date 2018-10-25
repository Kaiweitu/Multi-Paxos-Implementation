#pragma once
#include "Header.h"

using namespace std;

learner_data Learner::data;
Learner::Learner() {
    Learner::data.quorum = Server::addrs.size() / 2 + 1;
    _(dCout("[Learner] quorum is " + to_string(Learner::data.quorum));)
}

void Learner::start() {
    while (1) {
        string message_str = Server::learnerQue.pop();
        _(dCout("[Leaner] Receive: " + message_str);)
        stringstream ss(message_str);
        int type;
        ss >> type;
        int index = message_str.find_first_of(' ');
        string msg_str(message_str.begin() + index + 1, message_str.end());
        if (type == Learner::ACCEPT_MESSAGE) {
            acceptMsg msg = acceptMsg::deserialize(msg_str);
            std::thread t(handleAcceptMessage, std::ref(Learner::data), std::ref(msg));
        } else if (type == Learner::HEARTBEAT_MESSAGE) {
            // TODO: Heartbeat Message;
        }
    }
}

void Learner::handleAcceptMessage(learner_data &data, acceptMsg &msg) {
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(msg.port);
    client_addr.sin_addr.s_addr = msg.client_IP;

    unique_lock<mutex> lock(data.innerMutex);
    if (data.client_addrs.find(msg.client_ID) == data.client_addrs.end()) {
        data.client_addrs[msg.client_ID] = client_addr;
    }
    pair<int, int> command_id = make_pair(msg.client_ID, msg.seq);
    unique_lock<mutex> server_lock(Server::innerMutex);
    if (Server::logs[msg.slot].choosen) return;
    server_lock.unlock();

    if (data.cmd_map.find(msg.slot) != data.cmd_map.end()) {
        auto it = data.cmd_map[msg.slot].find(command_id);
        if (it != data.cmd_map[msg.slot].end()) {
            it -> second += 1;
            if (it -> second >= data.quorum) {
                
                string command = std::move(data.command_map[command_id]);

                _(dCout("[Leaner] Slot " + to_string(msg.slot) + " chose value: " + command);)
                // write into the log
                data.log.push_back(command);

                // Apply the log

            }
        } else {
            data.cmd_map[msg.slot][command_id] = 1;
        }
    }

}