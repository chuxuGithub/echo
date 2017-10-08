#include<unistd.h>
#include<sys/socket.h>
#include<strings.h>
#include<arpa/inet.h>
#include<stdio.h>
#include "Client.h"
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/resource.h>

int main(int argc,char **argv)
{
  int cliSocket[SOCKETSIZE];
  struct sockaddr_in servAddr;
  if(argc < 2)
  {
    printf("parameter is too few!\n");
    return 0;
  }
 // cliSocket = socket(AF_INET,SOCK_STREAM,0);
  bzero(&servAddr,sizeof(struct sockaddr_in));
  servAddr.sin_family = AF_INET;
  inet_pton(AF_INET,argv[1],&(servAddr.sin_addr.s_addr));
  servAddr.sin_port = htons((short)9877);
 
  struct rlimit limit,limit_new;
  if(getrlimit(RLIMIT_CORE,&limit) == 0)
  {
    limit_new.rlim_cur = limit_new.rlim_max = RLIM_INFINITY;
    if(setrlimit(RLIMIT_CORE,&limit)!=0)
    {
      limit_new.rlim_cur = limit_new.rlim_max = limit.rlim_max;
      setrlimit(RLIMIT_CORE,&limit_new);
    }  
  }

  if(connOp(cliSocket,servAddr) < 0)
  {
   printf("connect error!\n");
   return 0;
  }
  if(clientOp(cliSocket)<0)
  {
    printf("clientOp error!\n");
    return -1;
  }
  return 0;
}
