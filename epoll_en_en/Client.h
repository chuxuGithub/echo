#ifndef _CLIENT_H_
#define _CLIENT_H_
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

#define SOCKETSIZE 20
#define HeadSize 4
#define TestDataLen 10000
#define EchoTimes 40000

ssize_t clientOp(int cliSocket[]);
void setNonBlock(int fd);
ssize_t connOp(int cliSocket[SOCKETSIZE],struct sockaddr_in serAddr);
ssize_t readLine(FILE* fp,char * buf,size_t maxlen);//return the number of read
//ssize_t readSocket(int fd,char * buf,size_t maxlen);

int readFile(ifstream & in,string &s,int length);

class BufferRW
{
  public:
    int fd;
    unsigned int maxLen;
    unsigned int inIndex;
    unsigned int outIndex;
    bool headSend;
    bool headRecv;
    string totalString;
    vector<string> serverBuffer;//the first element isn't length
    BufferRW(int fdd,unsigned int inIndexx,unsigned int outIndexx,bool headSendd,bool headRecvv)
    {
      fd = fdd;
     // maxLen = maxLenn;
      inIndex = inIndexx;
      outIndex = outIndexx;
      headSend = headSendd;
      headRecv = headRecvv;
    }
};

int writeSocket(int fd,BufferRW &bufferRW);
int readSocket(int fd,BufferRW &bufferRW);
int readHead(BufferRW & bufferRW,int size);
int writeHead(BufferRW & bufferRW,unsigned int *headLen,int size);
int readn(int fd,char*buffer,int n);
int writen(int fd,char*buffer,int n);
int charToInt(char *ch,int size);


#endif
