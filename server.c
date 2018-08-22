#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <wait.h>

struct Point{
    int x;
    int y;
};

typedef void Sigfun(int signo);

Sigfun * siganl(int signo, Sigfun *fun)
{
    struct sigaction act;
    act.sa_handler = fun;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if(signo == SIGALRM) {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif  
    }
    else{
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif
    }
    if(sigaction(signo, &act, &act)<0)
        return SIG_ERR;
    return act.sa_handler;
}

void signhandler(int signo)
{
    pid_t cp;
    int stat;
    cp = wait(&stat);
    printf("child process %d terminaled!",cp);
    return;
}

int main()
{
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1){
        perror("create socket");
        return -1;
    }
    if(bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))== -1){
        perror("bind");
        return -1;
    }
    if(listen(listen_fd, 2) == -1) {
        perror("listen");
        return -1;
    } 
    
    while(1)
    {
        printf("start to accept socket\n");
        struct sockaddr_in client_addr;
        char addr[20] = {0};
        socklen_t len = sizeof(client_addr);
        int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &len);
        if(client_fd < 0){
            perror("accept");
            return -1;
        }
        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, addr, 20);
        printf("accept socket form :%s,port:%d\n",addr, ntohs(client_addr.sin_port));
        if(fork()==0){
            close(listen_fd);
            printf("accept from client , now waiting for input from client!");
            while(1){
                char buffer[1024] = {0};
                int nRead = 0;
                if((nRead = read(client_fd,buffer, 1024)) > 0){
                    printf("get from client:%s\n",buffer);
                    write(client_fd,buffer,nRead);
                    continue;
                }
                close(client_fd);
                break;
            }
            exit(0);
        }
        close(client_fd);

    }
    return 0;

}
