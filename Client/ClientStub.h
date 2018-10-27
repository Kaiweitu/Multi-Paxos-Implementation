#pragma once
#include "Header.h"

using namespace std;

class ClientStub {
private:
    static mutex innerMutex;
    static condition_variable cv;

    struct impl_t {
        int client_ID;
        int seq = 0;
        int port;
        uint32_t host;
        int current_sId = 0;
        vector<struct sockaddr_in> addrs;
        bool reply;
    };
    static impl_t impl;
    static void receiveReply();

    static const int CLIENT_REQUEST = 3;
    static const int TIMEOUT_SEC = 5;
public:
    ClientStub(int client_ID, int _seq, int port, const string &ip, const string &config);
    int sendMessage(const string &msg);
    
};



