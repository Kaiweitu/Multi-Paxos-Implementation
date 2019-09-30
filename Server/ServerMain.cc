#include "Header.h"
using namespace std;

int main(int argc, char **argv) {
    srand(time(0));
    //ios_base::sync_with_stdio(false);
    if (argc != 3) {
        exit(1);
    } 
    int sId = atoi(argv[1]);
    int skippedSlot = atoi(argv[2]);
    const string config_file = "service.config";
    ifstream ifs;
    ifs.open(config_file);
    
    int replica_num;
    ifs >> replica_num;
    vector<string> hosts;
    vector<int> ports;
    hosts.resize(replica_num);
    ports.resize(replica_num);
    
    for (int i = 0; i < replica_num; i++) {
        string s_ip;
        int port;
        int id;
        int nonsense;
        ifs >> id >> s_ip >> port >> nonsense;
        if (i == sId) LOSS_RATE = nonsense;
        hosts[id] = s_ip;
        ports[id] = port;
    }
    
    Server server(ports[sId], sId,  hosts[sId],hosts, ports, skippedSlot);
    server.start();
}