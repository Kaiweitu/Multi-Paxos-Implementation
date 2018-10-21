#pragma once
#include "Header.h"

using namespace std;

struct LogEntry{
    string data;
    int viewNum;
    bool choosen;
    bool accepted;
    bool applied;
};


template<class T>
class TSQueue{
private:
    mutex innerMutex;
    deque<T> innerDeque;
    condition_variable cv;
public:
    T pop() {
        unique_lock<mutex> lck(innerMutex, defer_lock);
        lck.lock();
        while (innerDeque.empty()) cv.wait(lck);
        T ret = innerDeque.front();
        innerDeque.pop_front();
        lck.unlock();
        return ret;
    };

    void push(T value) {
        innerMutex.lock();
        innerDeque.push_back(value);
        cv.notify_one();
        innerMutex.unlock();
    };
};

class Server {
private:
    static int myPort;
    static int sId;
    static string ip;
    
    static mutex innerMutex;
    static int viewNum;
    static vector<LogEntry> logs;
    
    static TSQueue<string> leaderQue;
    static TSQueue<string> acceptorQue;
    static TSQueue<string> learnerQue;

    static vector<string> hosts;
    static vector<int> ports;

    Leader mLeader;
    Learner mLearner;
    Acceptor mAcceptor;


public:
    Server(int _myPort, int _sId, string& _ip, vector<string>& _hosts, vector<int> _ports) {
        myPort = _myPort;
        sId = sId;
        ip = _ip;
        hosts = _hosts;
        ports = _ports;
    }

    void start();


};
