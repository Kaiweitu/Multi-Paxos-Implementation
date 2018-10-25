#pragma once
#include "Header.h"

using namespace std;

struct LogEntry{
    string data;
    int viewNum;
    bool choosen;
    bool accepted;
    bool applied;
    LogEntry(): data(""), viewNum(-1), choosen(false), accepted(false), applied(false) {
        
    }
};

template<class T>
class TSQueue{
private:
    mutex innerMutex;
    deque<T> innerDeque;
    condition_variable cv;
public:
    T pop() {
        unique_lock<mutex> lck(innerMutex);
        while (innerDeque.empty()) cv.wait(lck);
        T ret = innerDeque.front();
        innerDeque.pop_front();
        return ret;
    };

    void push(T value) {
        unique_lock<mutex> lck(innerMutex);
        innerDeque.push_back(value);
        cv.notify_one();
    };

    void makeEmpty() {
        unique_lock<mutex> lck(innerMutex);
        while (innerDeque.empty()) innerDeque.pop_front();
    }   
};

class Server {
private:
    static int myPort;
    static int sId;
    static string ip;
    
    static mutex innerMutex;
    static vector<LogEntry> logs;
    
    static TSQueue<string> leaderQue;
    static TSQueue<string> acceptorQue;
    static TSQueue<string> learnerQue;

    static mutex addrsMutex;
    static vector<struct sockaddr_in> addrs;

    static mutex maxViewMutex;
    static int maxViewNum;

    Leader mLeader;
    Learner mLearner;
    Acceptor mAcceptor;

    friend class Leader;
    friend class Learner;
    friend class mAcceptor;

    void initAddrs(const vector<string>& _hosts, const vector<int>& _ports);
public:
    Server(int _myPort, int _sId, string& _ip, const vector<string>& _hosts, const vector<int>& _ports) {
        maxViewNum = 0;
        myPort = _myPort;
        sId = sId;
        ip = _ip;
        logs.push_back(LogEntry());
        initAddrs(_hosts, _ports);
    }

    void start();

};
