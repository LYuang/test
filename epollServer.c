#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <error.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>

int main()
{
    struct sockaddr_in listen_addr;
    memset(&listen_addr, 0, sizeof(struct sockaddr_in));
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(54321);
    listen_addr.sin_family = AF_INET;

    int lfd = socket(PF_INET, SOCK_STREAM, 0);
    if(lfd < 0){
        perror("create socket!");
        return -1;
    }
    if(bind(lfd, (struct sockaddr*)&listen_addr, sizeof(struct sockaddr_in) <0)){
        perror("bind");
        return -1;
    }
    if(listen(lfd, 2)<0){
        perror("listen");
        return -1;
    }
    
    int efd = epoll_create(1024);
    if(efd < 0){
        perror("epoll_create");
        return -1;
    }
    struct epoll_event ev, events[1024];
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    if(epoll_ctl(efd, EPOLL_CTL_ADD, lfd, &ev) == -1){
        perror("epoll_ctl");
        return -1;
    }
    for(;;){
        int nfds = epoll_wait(efd, events, 1024, -1);
        if(nfds == -1){
            perror("epoll_wait");
            return -1;
        }
        for(int n=0;n<nfds;++n){
            if(events[n].data.fd == lfd){
                struct sockaddr_in client_addr; memset(&client_addr, 0, sizeof(struct sockaddr_in));
                socklen_t len = sizeof(struct sockaddr_in);
                int conn_sock = accept(lfd, (struct sockaddr*)&client_addr, &len);
                if(conn_sock < 0){
                    perror("accept");
                    continue;
                }
                setnonblocking(conn_sock);
                struct epoll_event epv;
                epv.data.fd = conn_sock;
                epv.events = EPOLLIN | EPOLLET;
                epoll_ctl(efd, EPOLL_CTL_ADD, conn_sock, &epv);
            }
            else{
                if(events[n].events & EPOLLIN){
                    char buff[1024]; memset(buff, 0, 1024);
                    int nread = read(events[n].data.fd, buff, 1024);
                    if(nread < 0){
                        perror("read");
                        continue;
                    }
                    else if(n == 0){
                        printf("client want to close the connection!\n");
                        epoll_ctl(efd, EPOLL_CTL_DEL, events[n].data.fd, NULL);
                        close(events[n].data.fd);
                        continue;
                    }
                    else{
                        printf("%s",buff);
                        write(events[n].data.fd, buff, nread);
                    }
                }
            }
        }
    }
    return 0;
}
