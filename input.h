/*************************************************************************
    > File Name: input.h
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Wed 23 May 2018 09:32:36 AM CST
 ************************************************************************/

#ifndef INPUT_H
#define INPUT_H

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "userlist.h"
#include "filelist.h"
#include "udp.h"
#include "tcp.h"

struct lsnInputArg{
	pthread_t tid_lsnUDP;
	pthread_t tid_lsnTCP;
};

extern char host[80];

void inputInit();
void* lsnInput(void* arg);
void showhelp();
void handleSendTo();
void handleSendfile();
void handleGetfile();
void handlervfile();
void handleExit();
//void* recvfile(void* arg);

#endif
