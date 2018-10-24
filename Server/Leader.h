#pragma once
#include "Header.h"

using namespace std;

class Leader {

public:
    Leader(){
    };
    void start();
    int findNextUnchosenLog(int curIndex);
    void handleCommand(const string& command, int& seq, int& cid, string& sentence);
    bool prepare(int unchosenSlot);
    void propose();
};