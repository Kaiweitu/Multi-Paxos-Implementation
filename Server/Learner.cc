#pragma once
#include "Header.h"

using namespace std;

mutex Learner::innerMutex;
learner_data Learner::data;
Learner::Learner() {}

void Learner::start() {
    data.quorum = Server::addrs.size() / 2 + 1;
    data.acceptor_vec.resize(Server::addrs.size());
    _(dCout("[Learner] quorum is " + to_string(data.quorum));)
    _(dCout("[Learner] clean files");)
    remove( ("data" + to_string(Server::sId) + ".txt").c_str() );
    thread(sendHbMessage).detach();
    while (1) {    
        string message_str = Server::learnerQue.pop();
        _(dCout("[Learner] Receive: " + message_str);)
        stringstream ss(message_str);
        int type;
        ss >> type;
        int index = message_str.find_first_of(' ');
        string msg_str(message_str.begin() + index + 1, message_str.end());
        if (type == ACCEPT_MESSAGE) {
            acceptMsg msg = acceptMsg::deserialize(msg_str);
            std::thread(handleAcceptMessage, std::ref(Learner::data), msg).detach();
        } else if (type == HEARTBEAT_MESSAGE) {
            // TODO: Heartbeat Message;
            learnerHeartBeatMsg msg = learnerHeartBeatMsg::deserialize(msg_str);
            std::thread(handleHbMessage, std::ref(Learner::data), msg).detach();
        } else if (type == SUCCESS_MESSAGE) {
            successMsg msg = successMsg::deserialize(msg_str);
            std::thread(handleSuccessMessage, std::ref(data), msg).detach();
        }
    }
}

void Learner::handleAcceptMessage(learner_data &data, acceptMsg msg) {
    // Check whether chosen
    unique_lock<mutex> server_lock(Server::innerMutex);
    if (Learner::checkChosen(msg.slot)) return;
    _(dCout("Handle the accept message: " + acceptMsg::serialize(msg));)
    unique_lock<mutex> lock(Learner::innerMutex);
    
    // Add client_addrs into the map if not exist
    /*
    if (data.client_addrs.find(msg.client_ID) == data.client_addrs.end()) {
        struct sockaddr_in client_addr;
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(msg.port);
        client_addr.sin_addr.s_addr = msg.client_IP;
        data.client_addrs[msg.client_ID] = client_addr;
        _(dCout("Client ID:" + to_string(msg.client_ID));) 
        _(dCout("IP:Port: " + to_string(client_addr.sin_port) + ":" + to_string(client_addr.sin_addr.s_addr));)
    }*/

    // <client_ID, seq> uniquely identify the message
    pair<int, int> command_id = make_pair(msg.client_ID, msg.seq);

    _(dCout("size: " + to_string(data.acceptor_vec.size()));)
    while (msg.slot >= data.acceptor_vec[msg.server_id].size()) {
        data.acceptor_vec[msg.server_id].push_back(slotEntry());
    }

    slotEntry &slot = data.acceptor_vec[msg.server_id][msg.slot];
    _(dCout("slot client id:" + to_string(slot.client_ID));)
    if (slot.client_ID == -1 || slot.view_num < msg.view_num) {
        slot.client_ID = msg.client_ID;
        slot.seq = msg.seq;
        slot.view_num = msg.view_num;
        slot.command = msg.command;
        _(dCout("Put into the slot" + msg.command);)
    }
    // Check whether majority
    _(dCout("Start voting");)
    map<pair<int, int>, int> vote;
    for (vector<slotEntry> &vec : data.acceptor_vec) {
        if (msg.slot < vec.size() && vec[msg.slot].client_ID != -1) {
            pair<int, int> msg_id = make_pair(vec[msg.slot].client_ID, vec[msg.slot].seq); 
            auto it = vote.find(msg_id);
            if (it == vote.end()) {
                vote[msg_id] = 1;
                it = vote.find(msg_id);
            } else {
                it -> second += 1; 
            }
            _(dCout("Current Vote: " + to_string(it -> second));)
            if (it -> second >= data.quorum) {
                _(dCout("[Learner] Slot " + to_string(msg.slot) + " chose value: " + vec[msg.slot].command);)
                // Update the logs data structure
                Server::logs[msg.slot].data = vec[msg.slot].command;
                Server::logs[msg.slot].seq = msg.seq;
                Server::logs[msg.slot].client_ID = msg.client_ID;
                Server::logs[msg.slot].port = msg.port;
                Server::logs[msg.slot].userIP = msg.client_IP;
                Server::logs[msg.slot].chosen = true;
                Server::logs[msg.slot].accepted = true;
                // Check whether we can apply the log
                Learner::applyMessage(data);
                _(dCout("[Learner] log size equals to " + to_string(Server::logs.size()));)
            }
        } 
    }
}

void Learner::handleHbMessage(learner_data &data, learnerHeartBeatMsg msg) {
    if (Server::findNextUnchosenLog(0) > msg.first_unchosen_index) {
        successMsg msg_body;
        msg_body.slot = msg.first_unchosen_index;
        unique_lock<mutex> server_lock(Server::innerMutex);
        const LogEntry &entry = Server::logs[msg.first_unchosen_index];
        msg_body.command = entry.data;
        msg_body.seq = entry.seq;
        msg_body.client_ID = entry.client_ID;
        msg_body.port = entry.port;
        msg_body.client_IP = entry.userIP;
        server_lock.unlock();
        
        ostringstream oss;
        oss << LEARNER << ' ' << SUCCESS_MESSAGE << ' ' << successMsg::serialize(msg_body); 
        string msg_str = oss.str();
        close(sendMessage(Server::addrs[msg.server_id], msg_str));
    }
}

void Learner::handleSuccessMessage(learner_data &data, successMsg msg){
    unique_lock<mutex> server_lock(Server::innerMutex);
    
    if (!Learner::checkChosen(msg.slot)){
        Learner::checkChosen(msg.slot);
        LogEntry &entry = Server::logs[msg.slot];
        entry.data = msg.command;
        entry.seq = msg.seq;
        entry.client_ID = msg.client_ID;
        entry.accepted = true;
        entry.chosen = true;
        entry.userIP = msg.client_IP;
        entry.port = msg.port;
        _(dCout("Slot " + to_string(msg.slot) + " Change to " + msg.command);
        )
        auto it = data.client_addrs.find(entry.client_ID);
        if (it == data.client_addrs.end()) {
            // Question here : same client_ID different address
            struct sockaddr_in client_addr;
            client_addr.sin_family = AF_INET;
            client_addr.sin_port = htons(msg.port);
            client_addr.sin_addr.s_addr 
            = msg.client_IP;
            data.client_addrs[entry.client_ID] = client_addr;
        }
        // hold two locks;
        unique_lock<mutex> lock(Learner::innerMutex);
        Learner::applyMessage(data);
    } else {
        const LogEntry &entry = Server::logs[msg.slot];
        if (entry.client_ID != msg.client_ID || entry.seq != msg.seq){
            _(  dCout( "current command" + entry.data);
                dCout( to_string(Server::sId) + " Inconsistency happen client_ID " +
                to_string(msg.client_ID) + " msg seq " + to_string(msg.seq));
                exit(1);
            )
        }
    }
}

// Hold server lock
bool Learner::checkChosen(int slot) {
    while ((size_t) slot >= Server::logs.size()) {
        Server::logs.push_back(LogEntry());
        dCout("Learner: Extend the log entry " + to_string(Server::logs.size() - 1));
        assert(!Server::logs.back().chosen);
    }
    if (Server::logs[slot].chosen) return true;
    return false;
}

void Learner::sendHbMessage() {
    while (1) {
        for (size_t idx = 0; idx < Server::addrs.size(); idx ++) {
            if (idx != (size_t) Server::myPort) {
                learnerHeartBeatMsg msg_body;
                msg_body.first_unchosen_index = Server::findNextUnchosenLog(0);
                msg_body.server_id = Server::sId;
                
                ostringstream oss;
                oss << LEARNER << ' ' << HEARTBEAT_MESSAGE << ' ' << learnerHeartBeatMsg::serialize(msg_body); 
                string msg = oss.str();
                close(sendMessage(Server::addrs[idx], msg));
            }
        }
        // Every 300 ms sending the message
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Need to hold two lock
void Learner::applyMessage(learner_data &data) {
    while (data.next_apply < Server::logs.size() && Server::logs[data.next_apply].chosen) {
        LogEntry &entry = Server::logs[data.next_apply]; 
        auto it = data.client_last_apply.find(entry.client_ID);
        if (it != data.client_last_apply.end() && it -> second >= entry.seq && entry.data != "") {
            data.next_apply ++;
            _(dCout("Client " + to_string(entry.client_ID) + " has duplicate message with seq: " + to_string(entry.seq));)
            continue;
        } else {
            data.client_last_apply[entry.client_ID] = entry.seq;
        }
        entry.applied = true;
        data.log.push_back(entry.data);
        
        // append to file
        fstream fs;
        fs.open("data" + to_string(Server::sId) + ".txt", fstream::in | fstream::out | fstream::app);
        fs << entry.data << endl;
        fs.close();

        _(dCout("[Learner] apply slot: " + to_string(data.next_apply) + " with value: " + entry.data);)
        string reply_msg = to_string(entry.seq) + ' ' + 
                            to_string(entry.client_ID);
        // server_lock.unlock();
        // Send reponse to the client
        struct sockaddr_in client_addr;
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(entry.port);
        client_addr.sin_addr.s_addr = entry.userIP;
        close(sendMessage(client_addr, reply_msg));
        // server_lock.lock();
        data.next_apply ++;
    }
}