#include "Client.h"
#include <ctime>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <map>
#include <vector>
#include <math.h>
using namespace std;

int readFile(ifstream &in,string &s,int length)
{
  char * buf = (char *)malloc(sizeof(char)*(length+1));
  in.get(buf,length+1); // 读取会添加'\0'，还会计入总字符
  //buf[length] = '\0';
  s = buf;
  cout << s.size() << endl;
  free(buf); //直接free导致程序崩溃，因为s=buf直接给的是指针(不会崩溃，给的不是指针)
  return length;
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

ssize_t connOp(int cliSocket[SOCKETSIZE],struct sockaddr_in serAddr)
{
  int i=0;
  for(i=0;i<SOCKETSIZE;i++)
  {
    cliSocket[i] = socket(AF_INET,SOCK_STREAM,0);
    if(connect(cliSocket[i],(struct sockaddr*)&serAddr,sizeof(struct sockaddr))<0)
    {
      printf("connect error!\n");     
      return -1;
    }
    setNonBlock(cliSocket[i]);
  }
  return 0;  
}

ssize_t clientOp(int cliSocket[])
{
  //map<int,int> timesMap;
  
  //record the time
  clock_t Begin,End; 
  //read the data
  string testdataString;
  ifstream in("../testdata.txt");
  if(readFile(in,testdataString,TestDataLen) != TestDataLen)
  {
    cout << "readFile error!" << endl;
    exit(-1);
  }
  
  struct epoll_event ev,events[SOCKETSIZE];
  int epollFd = epoll_create(SOCKETSIZE*2);
  {
    int i;
    for(i=0;i<SOCKETSIZE;i++)
    {
      ev.events = EPOLLOUT | EPOLLET;// in out 一起先触发out
     // ev.data.fd = cliSocket[i];//change to the point
      BufferRW *bufferTmp = new BufferRW(cliSocket[i],0,0,false,false);
      string headString = "0123";
      int lenTmp = TestDataLen;
      //strcpy((char *)headString.c_str(),(char*)&lenTmp);//不能用strcpy
      *((int*)headString.c_str()) = lenTmp;
      bufferTmp->totalString =  headString + testdataString;
      bufferTmp->maxLen = TestDataLen + 4;
      ev.data.ptr = bufferTmp;
      epoll_ctl(epollFd,EPOLL_CTL_ADD, cliSocket[i],&ev); 
      //timesMap.insert(pair<int,int>(cliSocket[i],2));
    }
  }
  Begin = clock();
  int times = 0;
  while(1)
  {
      int nfd;
      nfd = epoll_wait(epollFd,events,SOCKETSIZE,-1);
      int i;
      for(i=0;i<nfd;i++)
      {
        if(events[i].events & EPOLLOUT)
        {
           int fd = ((BufferRW*)events[i].data.ptr)->fd; 
           int count;
           BufferRW * bufferRWPtr = (BufferRW *)events[i].data.ptr;
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
            if((bufferRWPtr->maxLen) == bufferRWPtr->outIndex)
            {
              bufferRWPtr->maxLen = 0;
              bufferRWPtr->inIndex = 0;
              bufferRWPtr->outIndex = 0;
              bufferRWPtr->totalString = ""; 
              bufferRWPtr->serverBuffer.clear();
              ev.events = EPOLLIN | EPOLLET;
              ev.data.ptr = bufferRWPtr;
              epoll_ctl(epollFd,EPOLL_CTL_MOD,fd,&ev); 
              break;
            }
          } 
        }
        else if(events[i].events & EPOLLIN)
        {
          BufferRW * bufferRWPtr = (BufferRW*)events[i].data.ptr;
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
            if(bufferRWPtr->maxLen == bufferRWPtr->inIndex)//!!!!防止read再次读到0结束
            {
              break;
            }
          }
          //don't have data meaning maybe client tranlate data over
          if((bufferRWPtr->maxLen == bufferRWPtr->inIndex))
          {
            bufferRWPtr->headRecv = false;
            bufferRWPtr->headSend = false;
            bufferRWPtr->totalString = "";
            vector<string>& sBf = bufferRWPtr->serverBuffer;
            for(vector<string>::iterator it=sBf.begin();it!=sBf.end();it++)
            {
              bufferRWPtr->totalString += *it;
            }
            //cout << bufferRWPtr->totalString << endl;
            bufferRWPtr->serverBuffer.clear();
            string headString = "0123";
            int lenTmp = TestDataLen;
            //strcpy((char*)headString.c_str(),(char*)&lenTmp);//不能用strcpy
            (*(int*)headString.c_str()) = lenTmp;
            bufferRWPtr->totalString = headString + testdataString;
            bufferRWPtr->maxLen = TestDataLen + 4;
            ev.events = EPOLLOUT | EPOLLET;
            ev.data.ptr = bufferRWPtr;
            epoll_ctl(epollFd,EPOLL_CTL_MOD,fd,&ev);          
            //timesMap[fd] -= 1;    
            times++;        
          }
          if(times == EchoTimes)
          {
            End = clock();
            cout << "Toal time: "<< (double)(End-Begin) / CLOCKS_PER_SEC << "s" << endl;
            int i;
            for(i=0;i<SOCKETSIZE;i++)
            {
              close(cliSocket[i]);
            }          
            exit(0); 
          }
          /*if(timesMap[fd] == 0)
          {
            shutdown(fd,SHUT_WR);
          }*/
        }
      }
  }
}

int readHead(BufferRW & bufferRW,int size)//size = 4
{
  int fd = bufferRW.fd;
  int count;
  unsigned int headLen;
  if((count=readn(fd,(char *)&headLen,size)) != size) 
  {
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



int writeSocket(int fd,BufferRW & bufferRW)
{
  int count;//不能定义为无符号！！！
  if((count = write(fd,bufferRW.totalString.c_str()+bufferRW.outIndex,bufferRW.maxLen-bufferRW.outIndex)) < 0)
  {
    return -1;
  }
  bufferRW.outIndex += count;
  return count;
}

int writeHead(BufferRW & bufferRW,unsigned int *headLen,int size)//size = 4
 {
   int fd = bufferRW.fd;
   int count;
   if((count=writen(fd,(char *)headLen,size)) != size) 
   {
     printf("writen error!\n");
     exit(0);
   }
   return count;
 }

int readSocket(int fd,BufferRW& bufferRW)
{
  bool flag = false;
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
  if((count = read(fd,buffer,bufferRW.maxLen-bufferRW.inIndex)) < 0)//（+1）！！！
  {
    return -1;
  }
  buffer[count] = '\0';
  string s = buffer;
  free(buffer); //!!!!不能free
  bufferRW.serverBuffer.push_back(s);
  bufferRW.inIndex +=count;
  if(flag)
  {
    count += HeadSize;
  }
  return count; 
}

