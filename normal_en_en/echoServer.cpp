#include<sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "Server.h"
#include <iostream>
#include <strings.h>
using namespace std;

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
    if(fork()==0)
    {
        close(listenSocket);
        if(serverOp(serSocket) < 0)
        {
          cout << "serverOp error!" << endl;
          return -1;
        }
        return 0;
    }
    close(serSocket);
  }
}
