#pragma once
#include "Header.h"

using namespace std;

class Leader {
private:
    bool prepared;
public:
    Leader(): prepared(false) {
    }
    void start();
    void handleCommand();
};