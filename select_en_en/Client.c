#include "Client.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>

int max(int a,int b)
{
  if(a>b)
  {
    return a;
  }
  return b;
}

ssize_t clientOp(int cliSocket)
{
  char sendBuf[256],revBuf[256];
  int count;
  while(1)
  {
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(cliSocket,&fdset);
    FD_SET(fileno(stdin),&fdset);
    select(max(fileno(stdin),cliSocket)+1,&fdset,NULL,NULL,NULL);
    if(FD_ISSET(fileno(stdin),&fdset))
    {
      if((count=readLine(stdin,sendBuf,256))>0)
      {
         write(cliSocket,sendBuf,count);
      }     
      else if(count<0)
      {
        printf("read from stdin error!\n");   
        return -1;
      }
      else
      {
        //shutdown(cliSocket,SHUT_WR);
      }
    }
    if(FD_ISSET(cliSocket,&fdset))
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
