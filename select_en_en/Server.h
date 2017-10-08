#ifndef _SERVER_H_
#define _SERVER_H_
#include <sys/types.h>

#define HeadSize 4
//#define TestDataLen 100

ssize_t serverOp(int serSocket);

ssize_t readSocket(int fd,char* & headBuf,char *&  bodyBuf,int headSize);
int readHead(int fd,char *maxLen,int size);
int readn(int fd,char *buffer,int n);
int writeSocket(int fd,char* headBuf,char* bodyBuf,int headSize,int bodySize);
int writen(int fd,char *buffer,int n);


#endif
