#include<sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>
#include <stdlib.h>
#include "Server.h"
#include <strings.h>

#define FD_SETSIE 100

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
  setNonBlock(listenSocket);
  ServerOp(listenSocket);
  return 0;
  /* while(1)
  {
      struct sockaddr_in cliAddr;
      socklen_t cliLen = sizeof(struct sockaddr);
      int serSocket = accept(listenSocket,(struct sockaddr*)&cliAddr,&cliLen);
      if((count=readSocket(client[i],revBuf,256))<0)                          
        {                                                                            
          printf("server read from client error!\n");                           
          close(client[i]);                                                     
           return 0;                                                             
        }                                                                                         
        else if(count == 0)                                                     
        {
           sleep(1);                                                             
           close(client[i]);                                                            
        }                           
        write(client[i],revBuf,count);                                          
        revBuf[count-1]='\0';                                                   
        printf("client[%d]:%s count:%d\n",i,revBuf,count);
        if((--availfd) <= 0)
        {
          break;
        }
      }
    }
*/

/*  while(1)
  {
    struct sockaddr_in cliAddr;
    socklen_t cliLen = sizeof(struct sockaddr);
    int serSocket = accept(listenSocket,(struct sockaddr*)&cliAddr,&cliLen);
    pthread_t tid; 
    int * serSocket_moc = (int *)malloc(sizeof(int));
    pthread_create(&tid,NULL,echo,serSocket_moc);  
  }
*/
}
