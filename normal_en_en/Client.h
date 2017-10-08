#ifndef _CLIENT_H_
#define _CLIENT_H_
#include <sys/types.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
using namespace std;
ssize_t clientOp(int cliSocket);
ssize_t readLine(FILE* fp,char * buf,size_t maxlen);//return the number of read
ssize_t readSocket(int fd,char * buf,size_t maxlen);

int writeSocket(int fd,char *headAndBody,int size);
int writen(int fd,char *buffer,int n);

int readHead(int fd,int *maxLen,int size);//maxLen->需读取字符个数  size->头部长度
int readn(int fd,char*buffer,int n);
int readFile(ifstream &in,string &s,int length);
#endif
