#pragma once
#include "Header.h"

using namespace std;

mutex ClientStub::innerMutex;
condition_variable ClientStub::cv;
ClientStub::impl_t ClientStub::impl;
const int ClientStub::CLIENT_REQUEST;
const int ClientStub::TIMEOUT_SEC;

ClientStub::ClientStub(int _client_ID, int _port, const string &ip, const string &config) {
    impl.client_ID = _client_ID;
    impl.port = _port;
    impl.seq = 0;
    impl.current_sId = 0;
    struct sockaddr_in client_addr;
    inet_pton(AF_INET, ip.c_str(), &(client_addr.sin_addr));
    impl.host = client_addr.sin_addr.s_addr;
    
    // Read in the servers information
    ifstream ifs(config);
    int replica_num;
    ifs >> replica_num;
    impl.addrs.resize(replica_num);
    for (int i = 0; i < replica_num; i++) {
        string s_ip;
        int port;
        int id;
        ifs >> id >> s_ip >> port;

        struct sockaddr_in serv_addr;
        struct hostent *server;
        // server = gethostbyname(ip.c_str());
        // store this IP address in sa:
        inet_pton(AF_INET, s_ip.c_str(), &(serv_addr.sin_addr));
        //if (server == nullptr) {
        //    cerr << "ERROR no such host" << endl;
        //    exit(1);
        //}
        //bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        //bcopy((char *)server -> h_addr, 
        //    (char *)&serv_addr.sin_addr.s_addr, server->h_length);
        serv_addr.sin_port = htons(port);

        impl.addrs[id] = serv_addr;
    }
    thread(ClientStub::receiveReply).detach();
}

int ClientStub::sendMessage(const string &msg) {
    unique_lock<mutex> lock(innerMutex);
    string request = to_string(CLIENT_REQUEST) + ' ' + to_string(impl.seq) + ' ' + to_string(impl.client_ID) + ':' + msg + ':' + to_string(impl.host) + ' ' + to_string(impl.port);
    impl.reply = false;

    while (!impl.reply) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        connect(sock, (struct sockaddr*)&impl.addrs[impl.current_sId], sizeof(sockaddr_in));
        uint32_t sendSize = htonl(request.size());
        send(sock, &sendSize , sizeof(uint32_t), 0);
        send(sock, request.c_str(), request.size() + 1, 0);
        _(cout << "Send out the request:" << request << endl;)
        close(sock);
        while(!impl.reply) {
            if (cv.wait_for(lock, TIMEOUT_SEC * chrono::seconds(1)) == cv_status::timeout) {
                impl.current_sId = (impl.current_sId + 1) % impl.addrs.size();
                _(cout << "Timeout: resend the message to " << impl.current_sId << endl;)
                break;
            }
        }
    }

    impl.seq ++;   
    return 1;
}

void ClientStub::receiveReply() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;    
    struct timeval timeout;
    // timeout.tv_sec = 5;
    // timeout.tv_usec = 0;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
//     setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
    
    if (fd < 0) {
        cerr << "ERROR opening socket" << endl;
        exit(1);
    }

    struct sockaddr_in server_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    bzero((char *) &server_addr, sizeof(server_addr));
    
    // Initialize the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(impl.port);
    server_addr.sin_addr.s_addr = impl.host;
    
    // Bind the server address to the sockets
    if (bind(fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        cerr << "ERROR on binding" << endl;
        exit(1);
    }
    
    if (listen(fd, SOMAXCONN) < 0) {
        cerr << "ERROR　on listening" << endl;
        exit(1);
    }
    
    while (1) {
        // accept message from the client
        int client_fd = accept(fd, (struct sockaddr *) &cli_addr, &cli_len);

        uint32_t size;
        recv(client_fd, &size, sizeof(uint32_t), 0);
        size = ntohl(size);
        char buff[100];
        recv(client_fd, buff, size, MSG_WAITALL);
    
        string msg(buff, buff + size);
        stringstream ss(msg);
        int seq;
        int cid;
        ss >> seq >> cid;
        _(cout << "Receive reply: " << seq << ' ' << cid << endl;)

        unique_lock<mutex> lock(innerMutex);
        assert(seq <= impl.seq);
        if (cid == impl.client_ID && seq == impl.seq) {
            impl.reply = true;
            cv.notify_one();
        }
        //close(client_fd);
    }
}