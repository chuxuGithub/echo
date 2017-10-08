//两个问题：1：count 不能为无符号:2：read完后需判断是否全部读完，不然会导致下次read 0字符，使程序退出 
//3；服务器accept的套接字不能设置为in & out不然会先响应out，导致客户端没有数据来，服务器就发送数据了

#ifndef _SERVER_H_
#define _SERVER_H_
#include <vector>
#include <string>
using namespace std;
//ssize_t serverOp(int serSocket);
#define SOCKETSIZE 101
#define HeadSize 4

void* echo(void* argv);
int ServerOp(int listenFd);
void setNonBlock(int fd);

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
    vector<string> serverBuffer;//the first element is length
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

#else

#endif
