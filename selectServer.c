#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <arpa/inet.h>

int main()
{
    struct sockaddr_in listen_sock;
    memset(&listen_sock, 0, sizeof(struct sockaddr_in));
    listen_sock.sin_family = AF_INET;
    listen_sock.sin_port = htons(54321);
    listen_sock.sin_addr.s_addr = htonl(INADDR_ANY);

    int fd_listen  = socket(PF_INET, SOCK_STREAM, 0);
    if(fd_listen < 0){
        perror("create:");
        return -1;
    }
    if(bind(fd_listen, (struct sockaddr*)&listen_sock, sizeof(struct sockaddr))<0){
        perror("bind:");
        return -1;
    }
    if(listen(fd_listen, 2)<0){
        perror("listen:");
        return -1;
    }

    fd_set  read_fs;
    FD_ZERO(&read_fs);
    FD_SET(fd_listen, &read_fs);
    int maxn = fd_listen;
    fd_set  write_fs, expt_fs;
    FD_ZERO(&write_fs); FD_ZERO(&expt_fs);
    while(1){
        fd_set _rfs, _wfs, _efs;
        _rfs = read_fs; _wfs = write_fs; _efs = expt_fs;
        int n = select(maxn+1, &_rfs, &_wfs, &_efs, NULL);
        if(n < 0){
            perror("select:");
            return -1;
        }
        else if(n > 0){
            for(int i=0;i<maxn+1;++i){
                if(FD_ISSET(i, &_rfs)){
                    //can read 
                    if(i == fd_listen){
                        //can accept 
                        struct sockaddr_in client_add; 
                        memset(&client_add, 0 ,sizeof(struct sockaddr_in));
                        socklen_t len;
                        int  client_fd = accept(fd_listen, (struct sockaddr*)&client_add, &len);
                        if(client_fd < 0){
                            perror("accept:\n");
                            continue;
                        }
                        char addr[20]; memset(addr, 0, sizeof(addr));
                        inet_ntop(AF_INET, &client_add.sin_addr.s_addr, addr, sizeof(addr));
                        printf("accept client from :%s, %d\n", addr, ntohs(client_add.sin_port));
                        FD_SET(client_fd, &read_fs);
                        maxn = client_fd > maxn?client_fd:maxn;
                    }else{
                        char buff[1024];memset(buff, 0, sizeof(buff));
                        int n = read(i, buff, 1024);
                        if(n == 0){
                            //get fin from the client 
                            printf("client want to close the socket!\n");
                            FD_CLR(i, &read_fs); FD_CLR(i, &write_fs);FD_CLR(i,&expt_fs);
                            close(i);
                            continue;
                        }else if(n > 0){
                            //read contents from client 
                            printf("%s\n", buff);
                            write(i, buff, n);
                        }else{
                            perror("read:");
                            continue;
                        }
                    }
                }else if(FD_ISSET(i, &_wfs)){
                    //can write 

                }else if(FD_ISSET(i, &_efs)){
                    //exception happens
                }
            }
        }
    }
    return 0;
}
