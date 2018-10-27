#include "ClientStub.h"

int main(int argc, char ** argv) {
    ClientStub ct(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), "127.0.0.1", "service.config");
    for (int idx = 0; idx < 200; idx ++){
        ct.sendMessage( "Client " + to_string(atoi(argv[1])) + " test " + to_string(idx));
    }
    // ct.sendMessage( "Client " + to_string(atoi(argv[1])) + " test " + to_string(atoi(argv[2])));
}