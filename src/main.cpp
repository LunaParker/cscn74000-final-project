#include <iostream>
#ifdef _MSC_VER
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include "main.h"

int main() {
    return 0;
}
