#include<unistd.h>
#include<sys/socket.h>
#include<strings.h>
#include<arpa/inet.h>
#include<stdio.h>
#include "Client.h"
#include <netinet/in.h>
#include <sys/types.h>

int main(int argc,char **argv)
{
  int cliSocket;
  struct sockaddr_in servAddr;
  if(argc < 2)
  {
    printf("parameter is too few!\n");
    return 0;
  }
  cliSocket = socket(AF_INET,SOCK_STREAM,0);
  bzero(&servAddr,sizeof(struct sockaddr_in));
  servAddr.sin_family = AF_INET;
  inet_pton(AF_INET,argv[1],&(servAddr.sin_addr.s_addr));
  servAddr.sin_port = htons((short)9877);
  if(connect(cliSocket,(struct sockaddr*)&servAddr,sizeof(struct sockaddr)) < 0)
  {
   printf("connect error!\n");
   return 0;
  }
  if(clientOp(cliSocket)<0)
  {
    printf("clientOp error!\n");
    return -1;
  }
  close(cliSocket);
  return 0;
}
