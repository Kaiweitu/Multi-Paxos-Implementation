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
#include <chrono>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#include "Util.h"
#include "Acceptor.h"
#include "Leader.h"
#include "Learner.h"
#include "Server.h"


#define _(x) x

using namespace std;

const unsigned int TIMEOUTTIME = 2;
