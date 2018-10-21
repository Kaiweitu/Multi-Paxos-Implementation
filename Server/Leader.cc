#pragma once
#include "Header.h"

using namespace std;

void Leader::start() {
    while (true) {
        string command = Server::leaderQue.pop();
        handleCommand(command);
        if (!prepared) 
            prepare();
        propose();
    }
}