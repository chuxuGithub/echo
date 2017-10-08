#include "Server.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
using namespace std;

void* echo(void *argv)
{
  int count;
  int serSocket = (int)(*(int*)argv);
  free(argv);
  pthread_mutex_unlock(&mutex);
  pthread_detach(pthread_self());
  while(1)
  {
    char *headBuf;
    char *bodyBuf;
    if((count=readSocket(serSocket,headBuf,bodyBuf,HeadSize))<0)
    {
      printf("server read from client error!\n");
      close(serSocket);
      return NULL;   
    }
    else if(count == 0)
    {
      sleep(1);
      close(serSocket);
      return NULL;
    }
    bodyBuf[*(int*)headBuf] = 0;//!!!不知为什么会多读
    //cout << bodyBuf << endl;
    /*if(strlen(bodyBuf) != *(int*)headBuf)
    {
      cout << "error" << endl;
    }*/
    if(writeSocket(serSocket,headBuf,bodyBuf,HeadSize,*(int*)headBuf) < 0)
    {
      cout << "server read body from server error!" << endl;
      return NULL;
    }     
  }
}

ssize_t readSocket(int fd,char*& headBuf,char * &bodyBuf,int headSize)//headBuf bodyBuf 必须为引用
{
  headBuf = (char *)malloc(sizeof(char)*headSize);
  if(readHead(fd,headBuf,headSize) != headSize)
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
  bodyBuf[*(int *)headBuf] = '\0';
  return *(int*)headBuf+headSize;
}



int readHead(int fd,char *maxLen,int size)
{
  int count;
  if((count=readn(fd,(char *)maxLen,size)) != size)
  {
    if(count == 0)
    {
      sleep(1);
      close(fd);
      exit(1);
    }
    //printf("readn error!\n");
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
        //printf("read error!\n");
        return -1;
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
        printf("write error!\n");
        exit(0);
      }
    }
    left -= nwrite;
    tmp += nwrite;
  } 
  return n-left;
}


