#pragma once
#include "Header.h"

using namespace std;

int Server::myPort = 0;

void Server::start() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;    
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
    
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

    while (1) {
        // accept message from the client
        int client_fd = accept(fd, (struct sockaddr *) &cli_addr, &cli_len);
        // Read the port adn client ip
        string client_ip(inet_ntoa(cli_addr.sin_addr));
        int port = server_addr.sin_port;
        _(cout << "Receive request from " << client_ip << ":" << port << endl;)

    }
}
