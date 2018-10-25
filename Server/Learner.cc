#pragma once
#include "Header.h"

using namespace std;

mutex Learner::innerMutex;
Learner::Learner() {
    data.quorum = Server::addrs.size() / 2 + 1;
    _(dCout("[Learner] quorum is " + to_string(data.quorum));)
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
            learnerHeartBeatMsg msg = learnerHeartBeatMsg::deserialize(msg_str);
            std::thread t(handleHbMessage, std::ref(Learner::data), std::ref(msg));
        }
    }
}

void Learner::handleAcceptMessage(learner_data &data, acceptMsg &msg) {
    // Check whether chosen
    unique_lock<mutex> server_lock(Server::innerMutex);
    if (Learner::checkChosen(data, msg.slot)) return;
    unique_lock<mutex> lock(Learner::innerMutex);
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(msg.port);
    client_addr.sin_addr.s_addr = msg.client_IP;

    if (data.client_addrs.find(msg.client_ID) == data.client_addrs.end()) {
        data.client_addrs[msg.client_ID] = client_addr;
    }

    pair<int, int> command_id = make_pair(msg.client_ID, msg.seq);

    if (data.cmd_map.find(msg.slot) != data.cmd_map.end()) {
        auto it = data.cmd_map[msg.slot].find(command_id);
        if (it != data.cmd_map[msg.slot].end()) {
            it -> second += 1;
            if (it -> second >= data.quorum) {
                string command = data.command_map[command_id];
                _(dCout("[Leaner] Slot " + to_string(msg.slot) + " chose value: " + command);)
                // Modify the logs data structure
                Server::logs[msg.slot].data = command;
                Server::logs[msg.slot].seq = msg.seq;
                Server::logs[msg.slot].client_ID = msg.client_ID;
                Server::logs[msg.slot].chosen = true;
                Server::logs[msg.slot].accepted = true;
                data.cmd_map.erase(msg.slot);
                // Check whether we can apply the log
                Learner::applyMessage(data);
            }
        } else {
            data.cmd_map[msg.slot][command_id] = 1;
        }
    } else {
        data.cmd_map[msg.slot][command_id] = 1;
    }
}

void Learner::handleHbMessage(learner_data &data, learnerHeartBeatMsg &msg) {
    if (Server::findNextUnchosenLog(0) > msg.first_unchosen_index) {
        successMsg msg_body;
        msg_body.slot = msg.first_unchosen_index;
        unique_lock<mutex> server_lock(Server::innerMutex);
        const LogEntry &entry = Server::logs[msg.first_unchosen_index];
        msg_body.command = entry.data;
        msg_body.seq = entry.seq;
        msg_body.client_ID = entry.client_ID;
        server_lock.unlock();
        
        unique_lock<mutex> lock(Learner::innerMutex);
        msg_body.port = data.client_addrs[msg_body.client_ID].sin_port;
        msg_body.client_IP = data.client_addrs[msg_body.client_ID].sin_addr.s_addr;
        lock.unlock(); 
        ostringstream oss;
        oss << LEARNER << ' ' << Learner::SUCCESS_MESSAGE << ' ' << successMsg::serialize(msg_body); 
        string msg_str = oss.str();
        sendMessage(Server::addrs[msg.server_id], msg_str);
    }
}

void Learner::handleSuccessMessage(learner_data &data, successMsg &msg){
    unique_lock<mutex> server_lock(Server::innerMutex);
    if (!Learner::checkChosen(data, msg.slot)) {
        unique_lock<mutex> lock(Learner::innerMutex);
        LogEntry &entry = Server::logs[msg.slot];
        entry.data = msg.command;
        entry.seq = msg.seq;
        entry.client_ID = msg.client_ID;
        entry.accepted = true;
        entry.chosen = true;
        auto it = data.client_addrs.find(entry.client_ID);
        if (it == data.client_addrs.end()) {
            // Question here : same client_ID different address
            struct sockaddr_in client_addr;
            client_addr.sin_family = AF_INET;
            client_addr.sin_port = htons(msg.port);
            client_addr.sin_addr.s_addr = msg.client_IP;
            data.client_addrs[entry.client_ID] = client_addr;
        }
        // hold two locks;
        Learner::applyMessage(data);
    } else {
        const LogEntry &entry = Server::logs[msg.slot];
        if (entry.client_ID != msg.client_ID || entry.seq != msg.seq){
            throw runtime_error("Inconsistency happen");
        }
    }
}

bool Learner::checkChosen(learner_data &data, int slot) {
    if ((size_t) slot >= Server::logs.size()) {
        Server::logs.resize(slot + 1);
        return false;
    }
    else if (Server::logs[slot].chosen) return true;
}

void Learner::sendHbMessage() {
    while (1) {
        for (size_t idx = 0; idx < Server::addrs.size(); idx ++) {
            if (idx != (size_t) Server::myPort) {
                learnerHeartBeatMsg msg_body;
                msg_body.first_unchosen_index = Server::findNextUnchosenLog(0);
                msg_body.server_id = Server::sId;
                
                ostringstream oss;
                oss << LEARNER << ' ' << Learner::HEARTBEAT_MESSAGE << ' ' << learnerHeartBeatMsg::serialize(msg_body); 
                string msg = oss.str();
                sendMessage(Server::addrs[idx], msg);
            }
        }
        // Every 300 ms sending the message
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}

// Need to hold two lock
void Learner::applyMessage(learner_data &data) {
    while (data.next_apply < Server::logs.size() && Server::logs[data.next_apply].chosen) {
        Server::logs[data.next_apply].applied = true;
        data.log.push_back(Server::logs[data.next_apply].data);
        string reply_msg = to_string(Server::logs[data.next_apply].seq) + ' ' + 
                            to_string(Server::logs[data.next_apply].client_ID);
        // server_lock.unlock();
        // Send reponse to the client
        sendMessage(data.client_addrs[Server::logs[data.next_apply].client_ID], reply_msg);
        // server_lock.lock();
        data.next_apply ++;
    }
}