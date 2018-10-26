#include "ClientStub.h"

int main() {
    ClientStub ct(0, 8000, "127.0.0.1", "service.config");
    ct.sendMessage("test");
}