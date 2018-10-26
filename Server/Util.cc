#pragma once
#include "Header.h"

using namespace std;

const int MAXBUFFERSIZE = 200;

mutex cMutex;

void sendAndRecvMessage(struct sockaddr_in& addr, string& msg) {
    char buffer[MAXBUFFERSIZE];
    int replySize;
    int sock = sendMessage(addr, msg);
    recv(sock, &replySize, sizeof(int), MSG_WAITALL); replySize = ntohl(replySize);
    recv(sock, buffer, replySize, MSG_WAITALL);
    msg.assign(buffer, replySize); 
}

void dCout(const string& msg) {
    cMutex.lock();
    _(cout << msg << endl;)
    cMutex.unlock();
};

int sendMessage(struct sockaddr_in &addr, string &msg) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    sendMessageHelper(sock, msg);
    return sock;
}

void sendMessageHelper(int sock, const string& msg) {
    uint32_t sendSize = htonl(msg.size());
    send(sock, &sendSize , sizeof(int), 0);
    send(sock, msg.c_str(), sizeof(msg.c_str()), 0);
}