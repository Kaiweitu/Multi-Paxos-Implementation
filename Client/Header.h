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
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "clientStub.h"

#define _(x) x

using namespace std;

