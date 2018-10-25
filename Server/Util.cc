#pragma once
#include "Header.h"

using namespace std;

const int MAXBUFFERSIZE = 20;

int sendMessage(struct sockaddr_in& addr, string& msg) {
    char buffer[MAXBUFFERSIZE];
    int replySize;
    sock = socket(AF_INEF, SOCK_STREAM, 0);
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    send(sock, htonl(msg.size()), sizeof(int), 0);
    send(sock, msg.c_str(), sizeof(msg.c_str()), 0);
    recv(sock, replySize, sizeof(int), MSG_WAITALL); replySize = ntohl(replySize);
    recv(sock, buffer, replySize, MSG_WAITALL);
    msg.assign(buffer, replySize); 
}
