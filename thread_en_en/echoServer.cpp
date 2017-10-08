#include<sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <strings.h>
#include <sys/select.h>
#include <pthread.h>
#include <stdlib.h>
#include "Server.h"

pthread_mutex_t mutex;

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

  while(1)
  {
    struct sockaddr_in cliAddr;
    socklen_t cliLen = sizeof(struct sockaddr);
    int serSocket = accept(listenSocket,(struct sockaddr*)&cliAddr,&cliLen);
    pthread_t tid; 
    int * serSocket_moc = (int *)malloc(sizeof(int));
    pthread_mutex_init(&mutex,NULL);
    pthread_mutex_lock(&mutex);
    *serSocket_moc = serSocket;
    pthread_create(&tid,NULL,echo,serSocket_moc);  
  }
}
