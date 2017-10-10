#include "Server.h"
#include <sys/types.h>
#include <string.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <math.h>
#include <map>

int readHead(BufferRW & bufferRW,int size)//size = 4
{
  int fd = bufferRW.fd;
  int count;
  int headLen;
  if((count=readn(fd,(char *)&headLen,size)) != size) 
  {
    if(count == 0)
    {
	exit(0);
    }	    
    printf("readn error!\n");
    exit(0);
  }
  bufferRW.maxLen = headLen;
  return count;
}

int charToInt(char *ch,int size)//litter edge machine
{
  int i;
  int result = 0;
  for(i=0;i<size;i++)
  {
    result += ch[i]*pow(2,(size-i-1)*8);
  }
  return result;
}

int readn(int fd,char *buffer,int n)
{
  int left;
  int nread;
  char * tmp = buffer;
  left = n;
  while(left>0)
  {
    if((nread = read(fd,tmp,left)) < 0)
    {
      if(errno == EINTR || errno == EWOULDBLOCK)
      {
        continue;
      }
      else
      {
        //printf("read error!\n");
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

void setNonBlock(int fd)
{
  int flags; 
  if((flags=fcntl(fd,F_GETFL,0)) < 0)
  {
    printf("F_GETFL error!\n");
    return;
  }
  if(fcntl(fd,F_SETFL,flags | O_NONBLOCK) < 0)
  {
    printf("F_SETFL error!\n");
    return;
  }
}


int ServerOp(int listenFd)
{
  int epollFd = epoll_create(SOCKETSIZE*2);
  struct epoll_event ev,events[SOCKETSIZE];
  ev.events = EPOLLIN | EPOLLET;
  ev.data.fd = listenFd;
  epoll_ctl(epollFd,EPOLL_CTL_ADD,listenFd,&ev);
  for(;;)
  {
    int nfd = epoll_wait(epollFd,events,SOCKETSIZE,-1);
    int i;
    for(i=0;i<nfd;i++)
    {
      if(events[i].data.fd == listenFd)
      {
        struct sockaddr_in cliAddr;
        socklen_t cliLen = sizeof(struct sockaddr);
        int serSocket = accept(listenFd,(struct sockaddr *)&cliAddr,&cliLen);
        setNonBlock(serSocket);
        ev.events = EPOLLIN | EPOLLET;
        BufferRW *bufferTmp = new BufferRW(serSocket,0,0,false,false);
        ev.data.ptr = bufferTmp;
        epoll_ctl(epollFd,EPOLL_CTL_ADD,serSocket,&ev);
      }
      else if(events[i].events & EPOLLIN)
      {
        BufferRW* bufferRWPtr = (BufferRW*)events[i].data.ptr;
        int fd = bufferRWPtr->fd;
        int count;
        while(1)
        {
          if((count = readSocket(fd,*bufferRWPtr)) < 0)
          {
            if(errno == EINTR)
            {
              continue;
            }
            if(errno == EWOULDBLOCK)
            {
              break;
            }
            printf("server read from client error!\n");
            epoll_ctl(epollFd,EPOLL_CTL_DEL,fd,&events[i]);
            close(fd);         
          }
          else if(count == 0)
          {
            sleep(1);
            epoll_ctl(epollFd,EPOLL_CTL_DEL,fd,&events[i]);
            close(fd);
            break;
          }
          if(bufferRWPtr->maxLen == bufferRWPtr->inIndex)//!!!
          {
            break;
          }
        }
        //don't have data meaning maybe client tranlate data over
        if(bufferRWPtr->maxLen == bufferRWPtr->inIndex)
        {
          bufferRWPtr->totalString = "";
          vector<string>& sBf = bufferRWPtr->serverBuffer;
          for(vector<string>::iterator it=sBf.begin();it!=sBf.end();it++)
          {
            bufferRWPtr->totalString += *it;
          }
          bufferRWPtr->serverBuffer.clear();
          //cout << bufferRWPtr->totalString << endl;
          string headString = "0123";
          int lenTmp = bufferRWPtr->maxLen;//不加4，对面read是先读头部 
          //strcpy((char*)headString.c_str(),(char*)&lenTmp);//不能用strcpy，因为该函数的终止条件是'\0'会少复制一些数
          ((char*)headString.c_str())[0] = ((char*)&lenTmp)[0];//低地址存低位
          ((char*)headString.c_str())[1] = ((char*)&lenTmp)[1];
          ((char*)headString.c_str())[2] = ((char*)&lenTmp)[2];
          ((char*)headString.c_str())[3] = ((char*)&lenTmp)[3];
          bufferRWPtr->totalString = headString + bufferRWPtr->totalString;
          ev.events = EPOLLOUT | EPOLLET;
          ev.data.ptr = bufferRWPtr;
          if(epoll_ctl(epollFd,EPOLL_CTL_MOD,fd,&ev) == -1) 
          {
            cout << "epoll_ctl:mod error!" << endl;
            exit(1);
          }         
        }
      }
      else if(events[i].events & EPOLLOUT)
      {
        BufferRW * bufferRWPtr = (BufferRW*)events[i].data.ptr;
        int fd = bufferRWPtr->fd;
        int count;
        while(1)
        {
          if((count = writeSocket(fd,*bufferRWPtr)) <= 0)
          {
            if(count<0 && errno == EINTR)
            {
              continue;
            }
            else if(count < 0 && errno == EWOULDBLOCK)
            {
              break;
            }
            else
            {
              printf("server write to client error!\n");
              epoll_ctl(epollFd,EPOLL_CTL_DEL,fd,&ev);
              close(fd);
            }
          }
          //don't have place to store data must express data isn't over
          if((bufferRWPtr->maxLen+4) == bufferRWPtr->outIndex)
          {
            bufferRWPtr->maxLen = 0;
            bufferRWPtr->inIndex = 0;
            bufferRWPtr->outIndex = 0;
            bufferRWPtr->totalString = "";
            bufferRWPtr->headRecv = false;
            bufferRWPtr->headSend = false;
            ev.events = EPOLLIN | EPOLLET;
            ev.data.ptr = bufferRWPtr;//!!!!
            epoll_ctl(epollFd,EPOLL_CTL_MOD,fd,&ev); 
            break;
          }
        }        
      }
    }
  }
  return 0;
}

int writeSocket(int fd,BufferRW& bufferRW)
{
  int count;
  if((count = write(fd,bufferRW.totalString.c_str()+bufferRW.outIndex,bufferRW.maxLen+4-bufferRW.outIndex)) < 0)
  {
    return -1;
  }
  bufferRW.outIndex += count;
  return count;
}

int readSocket(int fd,BufferRW& bufferRW)
{
  bool flag = false;//阻止头部长度反复加
  if(!bufferRW.headRecv)
  {
    if(readHead(bufferRW,HeadSize) != HeadSize)
    {
      exit(1);
    }
    flag = true;
    bufferRW.headRecv = true;
  }
  int count;
  char *buffer = (char *)malloc(sizeof(char)*(bufferRW.maxLen-bufferRW.inIndex+1));
  if((count = read(fd,buffer,bufferRW.maxLen-bufferRW.inIndex)) < 0)//必须是maxlen+1不加1无法产生EWOULDBOLCK?->代替解决方法：read完后检查是否读完
  {
    return -1;
  }
  buffer[count] = '\0';
  string s = buffer;
  free(buffer);
  bufferRW.serverBuffer.push_back(s);
  bufferRW.inIndex +=count;
  if(flag)
  {
    count += HeadSize;//防止服务器当只收到头部时误认为count = 0而退出
  }
  return count; 
}

int writeHead(BufferRW & bufferRW,unsigned int *headLen,int size)//size = 4
 {
   int fd = bufferRW.fd;
   int count;
  // char* buffer = (char *)malloc(sizeof(char)*(size+1));
   if((count=writen(fd,(char *)headLen,size)) != size) 
   {
     printf("writen error!\n");
     exit(0);
   }
   return count;
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
      if((nwrite<0) && (errno == EINTR || errno == EWOULDBLOCK))
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

