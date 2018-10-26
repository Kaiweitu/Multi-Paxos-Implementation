#include "ClientStub.h"

int main() {
    ClientStub ct(0, 8000, "localhost", "service.config");
    ct.sendMessage("test");
}