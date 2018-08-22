#include "util.h"
#include "stdio.h"

void setnonblocking(int sock){
    int opts;
    opts = fcntl(sock, F_GETFL);
    if(opts<0){
        perror("fcntl");
    }
    opts = opts | O_NONBLOCK;
    if(fcntl(sock, F_SETFL, opts)<0){
        perror("fcntl");
    }
}
