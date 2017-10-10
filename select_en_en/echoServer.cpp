#include<sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "Server.h"
#include <sys/select.h>
#include <strings.h>
#include <iostream>
using namespace std;

#define HeadSize 4

#define FD_SETSIE 1024

int main(int argc,char** argv)
{
  int listenSocket;
  struct sockaddr_in serAddr;
  listenSocket = socket(AF_INET,SOCK_STREAM,0);
  bzero(&serAddr,sizeof(struct sockaddr_in));
  serAddr.sin_family = AF_INET;
  serAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serAddr.sin_port = htons((short)9877);
  bind(listenSocket,(struct sockaddr*)&serAddr,sizeof(struct sockaddr));
  listen(listenSocket,20000);
  
  //select
  int maxfd=listenSocket;
  fd_set fdset;
  FD_ZERO(&fdset);
  FD_SET(listenSocket,&fdset);
  int client[FD_SETSIE];
  int maxi = -1;
  int i;
  for(i=0;i<FD_SETSIE;i++)
  {
    client[i] = -1;
  }
  while(1)
  {
    int availfd = select(maxfd+1,&fdset,NULL,NULL,NULL);
    if(FD_ISSET(listenSocket,&fdset))
    {
      struct sockaddr_in cliAddr;
      socklen_t cliLen = sizeof(struct sockaddr);
      int serSocket = accept(listenSocket,(struct sockaddr*)&cliAddr,&cliLen);
      int i;
      for(i=0;i<FD_SETSIE;i++)
      {
        if(client[i]<0)
        {
          client[i] = serSocket;
          break;
        }
      }
      if(i==FD_SETSIE)
      {
        printf("too many connet!\n");
        return -1;
      }
      if(maxi<i)
      {
        maxi = i;
      }
      if(serSocket>maxfd)
      {
        maxfd = serSocket;
      }
      if((--availfd) <= 0)
      {
        /********!!!!!!!!!!*/
        for(i=0;i<=maxi;i++)
       {
          if(client[i]>0)
         {
            FD_SET(client[i],&fdset);
          }
        }
        /************!!!!!!!!!!*/
        FD_SET(listenSocket,&fdset);
        continue;
      }
    }
    int i;
    for(i=0;i<=maxi;i++)
    {
      if(client[i]<0)
      {
        continue;
      }
      if(FD_ISSET(client[i],&fdset))
      {
        int count;                                                              
        char  * headBuf;
        char * bodyBuf;
        if((count=readSocket(client[i],headBuf,bodyBuf,HeadSize))<0)                          
        {                                                                            
          printf("server read from client error!\n");                           
          FD_CLR(client[i],&fdset);
          client[i] = -1;
          close(client[i]);                                                     
           return 0;                                                             
        }                                                                                         
        else if(count == 0)                                                     
        {
           client[i] = -1;
           FD_CLR(client[i],&fdset);
           sleep(1);                                                             
           close(client[i]);                                                            
        }
        if(writeSocket(client[i],headBuf,bodyBuf,HeadSize,*(int*)headBuf) < 0)
        {
          cout << "server write body from client error!" << endl;
        }
        //printf("client[%d]:%s count:%d\n",i,revBuf,count);
        if((--availfd) <= 0)
        {
          break;
        }
      }
    }

    /********************/
    for(i=0;i<=maxi;i++)
    {
      if(client[i]>0)
      {
        FD_SET(client[i],&fdset);
      }
    }
    FD_SET(listenSocket,&fdset);
    /********************/
  }
}
