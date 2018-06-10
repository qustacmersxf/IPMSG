/*************************************************************************
    > File Name: tcp.h
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Sun 27 May 2018 07:37:08 AM CST
 ************************************************************************/

#ifndef TCP_H
#define TCP_H

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <pthread.h>
#include "udp.h"
//#include "input.h"
#include "filelist.h"

#define MAXLINE 80
#define SERV_PORT 2425

extern int tcpsockfd;
extern char host[80];

void TCPInit();
void* lsnTCP(void* arg);
void* sendfile(void* arg);
void* recvfile(void* arg);
void parsenum(char* buf, int* fnum, int* pkgnum, int* offset);

#endif
