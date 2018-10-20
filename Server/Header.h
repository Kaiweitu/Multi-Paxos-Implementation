#pragma once
#include <mutex>
#include <thread>
#include <condition_variable>

#include <deque>
#include <vector>
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