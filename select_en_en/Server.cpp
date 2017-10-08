#include "Server.h"
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <iostream>
#include <fstream>
using namespace std;

ssize_t serverOp(int serSocket)
{
  int count;
  for(;;)
  {      
    char *headBuf;
    char *bodyBuf;    
    if(readSocket(serSocket,headBuf,bodyBuf,HeadSize) < 0) 
    {
      cout << "server read head from client error!" << endl;
      return -1;
    }
    //cout << bodyBuf << endl;
    if(writeSocket(serSocket,headBuf,bodyBuf,HeadSize,*(int*)headBuf) < 0)
    {
      cout << "server write body to client error!" << endl;
      return -1;
    }     
  }
  return 0;
}


ssize_t readSocket(int fd,char*& headBuf,char * &bodyBuf,int headSize)//headBuf bodyBuf 必须为引用
{
  headBuf = (char *)malloc(sizeof(char)*headSize);
  int count;
  if((count=readHead(fd,headBuf,headSize))==0)
  {
    return 0;    
  }      
  if(count != headSize)
  {
    cout << "readHead error!" << endl;
    return -1;
  } 
  headBuf = (char *)realloc(headBuf,sizeof(char)*(*(int*)headBuf+1+headSize));
  if(headBuf == NULL)
  {
    cout << "error realloc memory!" << endl;
  }
  bodyBuf = (char *)(headBuf + headSize);
  if(readn(fd,bodyBuf,*(int *)headBuf) != *(int *)headBuf)
  {
    cout << "readn error!" << endl;
    return -1;
  }
  bodyBuf[*(int*)headBuf] = '\0';
  return *(int *)headBuf+headSize;
}



int readHead(int fd,char *maxLen,int size)
{
  int count;
  if((count=readn(fd,(char *)maxLen,size)) != size)
  {
    if(count == 0)
    {
      return 0;
    }
    printf("readn error!\n");
    exit(0);
  }  
  return count;
}

int readn(int fd,char*buffer,int n)
{
  int left;
  int nread;
  char * tmp = buffer;
  left = n;
  while(left>0)
  {
    if((nread = read(fd,tmp,left)) < 0)
    {
      if(errno == EINTR)
      {
        continue;
      }
      else
      {
        printf("read error!\n");
        exit(0);
      }
    }
    else if(nread == 0)
    {
      break;
    }
    left -= nread;
    tmp += nread;
  } 
  return n-left;
}

int writeSocket(int fd,char* headBuf,char* bodyBuf,int headSize,int bodySize)
{
  int count;
  if((count = writen(fd,(char *)headBuf,headSize+bodySize)) != (headSize+bodySize))
  {
    cout << "write head+body error!" << endl;
    return -1;
  }  
  /*if(writen(fd,bodyBuf,bodySize) != bodySize)
  {
    cout << "write body error!" << endl;
    return -1;
  }*/
  //cout << bodyBuf << endl;
  free(headBuf);
  return headSize+bodySize;
}

int writen(int fd,char *buffer,int n)
{
  int left;
  int nwrite;
  char * tmp = buffer;
  left = n;
  while(left>0)
  {
    if((nwrite = write(fd,tmp,left)) <= 0)
    {
      if(nwrite<0 && errno == EINTR)
      {
        continue;
      }
      else
      {
        //printf("write error!\n");
        exit(0);
      }
    }
    left -= nwrite;
    tmp += nwrite;
  } 
  return n-left;
}


