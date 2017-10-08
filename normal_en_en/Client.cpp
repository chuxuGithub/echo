#include "Client.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fstream>
#include <stdlib.h>
using namespace std;

#define TestDataLen 4

ssize_t clientOp(int cliSocket)
{
  char sendBuf[256],revBuf[256];
  int count = 4;
  //read the data
  string testdataString;
  ifstream in("../testdata.txt");
  if(readFile(in,testdataString,TestDataLen) != TestDataLen)
  {
    cout << "readFile errot!\n" <<endl;
    exit(-1);
  }
  for(int i=0;i<2;i++)
  {      
   sendBuf[0] = 'h';
   sendBuf[1] = 'e';
   sendBuf[2] = '\0';
   sendBuf[3] = 'l';
//   int num = 1140850756;
   if(writeSocket(cliSocket,sendBuf,count))
   {
    
   }
    if(readSocket(cliSocket,revBuf,256)<0)
    {
       printf("client read from server error!\n");
       return -1;
    }
    fputs(revBuf,stdout);
    fputs("\n",stdout);
  }
  return 0;
}

ssize_t readLine(FILE* fp,char * buf,size_t maxlen)
{
  if(fgets(buf,maxlen,fp)==NULL)
  {
   // while(1);
    printf("client exit!\n");
    return -1;
  }
 //
  if(strlen(buf) == 1)
  {
    return -1;
  }
  //
  return strlen(buf);
}


ssize_t readSocket(int fd,char * buf,size_t maxlen)
{
  int count = 0;
  size_t rc;
  char c;
again:
  while((rc=read(fd,&c,1))==1 && count < maxlen-1)
  {
    if(c!='\n')
    {
      buf[count] = c;
      count++;
    }
    else
    {
      break;
    }
  }
   if(rc == 0)
  {
    buf[count] = '\0';
    return count;
  }
  if(rc == -1)
  {
    if(errno == EINTR)
      goto again;
    return -1;
  }
  buf[count] = '\0';
  return count;
}

int readFile(ifstream &in,string &s,int length)
{
  char * buf = (char *)malloc(sizeof(char)*(length+1));
  in.get(buf,length+1); // 读取会添加'\0'，还会计入总字符
  //buf[length] = '\0';
  s = buf;
  cout << s.size() << endl;
  free(buf); 
  return length;
}



int readHead(int fd,int *maxLen,int size)
{
  int count;
  if((count=readn(fd,(char *)maxLen,size)) != size)
  {
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
    left -= nread;
    tmp += nread;
  } 
  return n-left;
}

int writeSocket(int fd,char* headAndBody,int size)
{
  int count;
  if((count = writen(fd,headAndBody,size)) != size)
  {
    return -1;
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


