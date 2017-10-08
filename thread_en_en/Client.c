#include "Client.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>

int max(int a,int b)
{
  if(a>b)
  {
    return a;
  }
  return b;
}

void* copyto(void*argv)
{
  int count;
  char sendBuf[256];
  int cliSocket = (int)(*(int*)argv);
  while((count=readLine(stdin,sendBuf,256)) > 0)
  {
    write(cliSocket,sendBuf,count);
  }
  if(count<0)
  {
    printf("read from stdin error!\n");
    free(argv);
    return NULL;
  }
  free(argv);
  shutdown(cliSocket,SHUT_WR);
  return NULL;
}

ssize_t clientOp(int cliSocket)
{
  char sendBuf[256],revBuf[256];
  int count;
  pthread_t tid;
  int * cliSocket_moc = (int *)malloc(sizeof(int));
  pthread_create(&tid,NULL,copyto,cliSocket_moc);
  while(1)
  {
      if((count=readSocket(cliSocket,revBuf,256))<0)                                      
      {
        printf("read from socket error!\n");
        return -1;
      }
      else if(count == 0)
      {
        return -1;
      }
      revBuf[count] = '\0';    
      fputs(revBuf,stdout);      
      fflush(stdout);
    }
}

ssize_t readLine(FILE* fp,char * buf,size_t maxlen)
{
  if(fgets(buf,maxlen,fp)==NULL)
  {
    printf("client exit!\n");
    return -1;
  }
  return strlen(buf);
}


ssize_t readSocket(int fd,char * buf,size_t maxlen)
{
  int count = 0;
  if((count=read(fd,buf,maxlen))<0)
  {
    return -1;
  }
  return count;
}
