#ifndef _CLIENT_H_
#define _CLIENT_H_
#include <sys/types.h>
#include <stdio.h>
ssize_t clientOp(int cliSocket);
ssize_t readLine(FILE* fp,char * buf,size_t maxlen);//return the number of read
ssize_t readSocket(int fd,char * buf,size_t maxlen);
#endif
