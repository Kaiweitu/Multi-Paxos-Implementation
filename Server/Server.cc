#pragma once
#include "Header.h"

using namespace std;

int Server::myPort;
int Server::sId;
int Server::maxViewNum;
int Server::skippedSlot;


string Server::ip;

mutex Server::innerMutex;
vector<LogEntry> Server::logs;
TSQueue<string> Server::leaderQue;
TSQueue<string> Server::acceptorQue;
TSQueue<string> Server::learnerQue;

vector<struct sockaddr_in> Server::addrs;

mutex Server::maxViewMutex;

int Server::findNextUnchosenLog(int curIndex) {
    unique_lock<mutex> iMutex(Server::innerMutex);
    
    while (curIndex >= Server::logs.size()) {
        Server::logs.push_back(LogEntry());
    }
    
    for (size_t i = curIndex; i < Server::logs.size(); ++ i)
        if (!Server::logs[i].chosen) return i;
    
    Server::logs.push_back(LogEntry());

    return Server::logs.size() - 1;
}


void Server::initAddrs(const vector<string>& _hosts, const vector<int>& _ports) {
    for (size_t i = 0; i < _hosts.size(); ++ i) {
        struct sockaddr_in servAddr;
        memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        servAddr.sin_port = htons(_ports[i]);
        inet_pton(AF_INET, _hosts[i].c_str(), &servAddr.sin_addr);
        addrs.push_back(servAddr);
    }
}

void Server::start() {
        
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;    
    struct timeval timeout;
    // timeout.tv_sec = 10;
    // timeout.tv_usec = 0;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    // setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
    
    if (fd < 0) {
        cerr << "ERROR opening socket" << endl;
        exit(1);
    }

    struct sockaddr_in server_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    bzero((char *) &server_addr, sizeof(server_addr));
    
    // Initialize the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(myPort);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    // Bind the server address to the sockets
    if (bind(fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        cerr << "ERROR on binding" << endl;
        exit(1);
    }
    
    if (listen(fd, SOMAXCONN) < 0) {
        cerr << "ERROR　on listening" << endl;
        exit(1);
    }
    
    thread leader_t(Leader::start);
    thread learner_t(Learner::start);
    thread acceptor_t(Acceptor::start);

    while (1) {
        // accept message from the client
        _(dCout("Try to accept message");)
        int client_fd = accept(fd, (struct sockaddr *) &cli_addr, &cli_len);
        if (client_fd < 0) {

             char buffer[256];
             char *errorMessage = strerror_r(errno, buffer, 256);
             _(
                string msg(buffer);
                dCout("Accept Fail: " + msg);
                )
            continue;
        }
        // Read the port adn client ip
        string client_ip(inet_ntoa(cli_addr.sin_addr));
        // unsigned int client_ip = cli_addr.sin_addr.s_addr;
        int client_port = server_addr.sin_port;
        _(dCout("Receive request from " + client_ip + ":" + to_string(client_port));)
        
        // Receive the size of the message
        uint32_t size;
        recv(client_fd, &size, sizeof(uint32_t), MSG_WAITALL);
        
        size = ntohl(size);
        _(dCout("Request size: " + to_string(size));)
        // Receive the message
        unique_ptr<char[]> message_char(new char[size]);
        recv(client_fd, message_char.get(), size, MSG_WAITALL);
        string message_str(message_char.get(), message_char.get() + size);
        _(dCout("Receive message: " + message_str);)
        
        stringstream ss(message_str);
        int message_owner;
        ss >> message_owner;
        int index = message_str.find_first_of(' ');
        string msg(message_str.begin() + index + 1, message_str.end());

        if (message_owner == CLIENT_REQUEST) {
            _(dCout("Push request message to the leader queue: " + message_str + '\n');)
            leaderQue.push(msg);
            close(client_fd);
        } else if (message_owner == ACCEPTOR) {
            // TODO: append file descriptor;
            _(dCout("Push message to the acceptor queue: " + msg);)
            acceptorQue.push(msg + ';' + to_string(client_fd));
        } else if (message_owner == LEARNER) {
            _(dCout("Push message to the learner queue: " + msg);)
            learnerQue.push(msg);
            close(client_fd);
        }
    }
}
