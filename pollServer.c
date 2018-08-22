#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>
#include <error.h>
#include <errno.h>

int main()
{

    struct sockaddr_in listen_addr;
    memset(&listen_addr, 0, sizeof(struct sockaddr_in));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(54321);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0){
        perror("create socket:");
        return -1;
    }
    
    if(bind(listenfd, (struct sockaddr*)&listen_addr, sizeof(struct sockaddr_in))<0){
        perror("bind:");
        return -1;
    }

    if(listen(listenfd, 2)<0){
        perror("listen:");
        return -1;
    }

    struct  pollfd  pfds[1024];
    for(int i=0;i<1024;++i){
        pfds[i].fd = -1;
    }
    int num = 0;
    pfds[num].fd = listenfd;
    pfds[num].events = POLLIN | POLLRDNORM;
    while(1){
        int n = poll(pfds, num+1, 0);
        if(n<0){
            if(errno == EINTR){
                continue;
            }
            perror("poll:");
            return -1;
        }
        else if(n == 0){
            continue;
        }else{
            for(int i=0;i<num+1;++i){
                if(pfds[i].fd != -1){
                    if(pfds[i].revents & (POLLIN | POLLRDNORM)){
                        //can read 
                        if(pfds[i].fd == listenfd) //accept
                        {
                            struct sockaddr_in client_addr; memset(&client_addr, 0, sizeof(struct sockaddr));
                            socklen_t len = sizeof(struct sockaddr_in);
                            int cfd = accept(listenfd, (struct sockaddr*)&client_addr, &len);
                            if(cfd < 0){
                                perror("accept");
                                continue;
                            }else{
                                if(num == 1024){
                                    printf("to many fd!");
                                    continue;
                                }
                                for(int j=0;j<1024;++j)
                                {
                                    if(pfds[j].fd == -1){
                                        pfds[j].fd = cfd;
                                        pfds[j].events = POLLIN | POLLRDNORM;
                                        num++;
                                        char addbuf[20]; memset(addbuf, 0, 20);
                                        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, addbuf, 21);
                                        printf("accept connect from %s,%d\n",addbuf, ntohs(client_addr.sin_port));
                                        break;
                                    }
                                }
                            }
                        }else{
                            char buf[1024]; memset(buf, 0, 1024);
                            int nread = read(pfds[i].fd, buf, 1024);
                            if(nread == 0){
                                printf("client going to close the connection!\n");
                                close(pfds[i].fd);
                                pfds[i].fd = -1;
                                continue;
                            }else{
                                printf("%s", buf);
                                write(pfds[i].fd, buf, nread); //block write, wait until write succeed.
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}
