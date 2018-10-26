#include "ClientStub.h"

int main() {
    ClientStub ct(0, 8000, "127.0.0.1", "service.config");
    for (int idx = 0; idx < 3; idx ++){
        ct.sendMessage("test" + to_string(idx));
    }

}