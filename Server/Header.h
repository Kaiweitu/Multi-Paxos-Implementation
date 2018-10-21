#pragma once
#include <mutex>
#include <thread>
#include <condition_variable>

#include <deque>
#include <vector>
#include <utility>
#include <map>
#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Acceptor.h"
#include "Leader.h"
#include "Learner.h"
#include "Server.h"
#include "Util.h"

#include "Server.h"

#define _(x) x

using namespace std;
// receiver type
const int LEADER = 0;
const int ACCEPTOR = 1;
const int LEARNER = 2;
const int CLIENT_REQUEST = 3;

// message type
const int MESSAGE_PREPARE = 0;
const int MESSAGE_PREPOSE = 1;

