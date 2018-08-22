#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main()
{
    int cn = 1;
    while(cn--)
    {
       int fd = socket(PF_INET, SOCK_STREAM, 0);
       if(fd < 0){
           perror("create socket:");
           return -1;
       }

       struct sockaddr_in server_addr;
       memset(&server_addr,  0, sizeof(struct sockaddr_in));
       server_addr.sin_family = AF_INET;
       inet_pton(AF_INET, "192.168.1.179", &server_addr.sin_addr);
       server_addr.sin_port = htons(54321);
       socklen_t len = sizeof(server_addr);

       if(connect(fd, (struct sockaddr*)&server_addr, len) == -1) {
            perror("connect");
            return -1;
       }
       printf("connect succeed!\n");
       struct sockaddr_in peerAddr;
       socklen_t len_t = sizeof(struct sockaddr_in);
       getpeername(fd,(struct sockaddr*)&peerAddr,&len_t);

       char addr[20] = {0};
       inet_ntop(PF_INET, &peerAddr.sin_addr.s_addr, addr, sizeof(addr));
       printf("connect to server:%s,port:%d:num:%d\n", addr, ntohs(peerAddr.sin_port),1000-cn);
       char sendline[1024],recv[1024];
       memset(sendline,0,1024*sizeof(char));
       memset(recv, 0, 1024*sizeof(char));
       while(fgets(sendline,1024,stdin) != NULL){
           write(fd, sendline, strlen(sendline));
           memset(recv, 0, 1024*sizeof(char));
           if(read(fd,recv,1024) == 0){
               printf("server terminaled!");
           }
           fputs(recv,stdout);
       }
   }
  return 0;
}
