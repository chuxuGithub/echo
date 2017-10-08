#ifndef _SERVER_H_
#define _SERVER_H_
#include <sys/types.h>
#include <pthread.h>
#define HeadSize 4

extern pthread_mutex_t mutex;

void* echo(void* argv);

ssize_t readSocket(int fd,char* & headBuf,char *&  bodyBuf,int headSize);
int readHead(int fd,char *maxLen,int size);
int readn(int fd,char *buffer,int n);
int writeSocket(int fd,char* headBuf,char* bodyBuf,int headSize,int bodySize);
int writen(int fd,char *buffer,int n);

#endif
