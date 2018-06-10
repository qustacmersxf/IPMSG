/*************************************************************************
    > File Name: udp.h
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Thu 10 May 2018 04:18:17 PM CST
 ************************************************************************/

#ifndef UDP_H
#define UDP_H

#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include "myinclude.h"
#include "ipmsg.h"
#include "userlist.h"
#include "filelist.h"

struct UDPPackage{
	char version[20];
	unsigned long long num;
	char name[80];
	char host[80];
	unsigned long long cmd;
	char info[1024];
};

struct lsnUDPArg{
	int lsnSocket;
	struct sockaddr_in recvAddr;
};

extern struct userlist* ulist;
extern char un[80];
extern struct in_addr ip_br;
extern char name[80];

int UDPinit(struct lsnUDPArg* arg);
void* lsnUDP(void* arg);
int sendUDP(unsigned long long pkgnum, unsigned int cmd, char* info, int len, struct in_addr dstip);
int getbuflen(char* buf, int size);
void analysis(char* buf, struct UDPPackage* package);
void makepackage(char* buf, struct UDPPackage* package);
int addusr(struct UDPPackage* package, struct in_addr ip);
int reply(struct in_addr ip);
int rmusr(struct in_addr ip);
void displaymsg(struct UDPPackage* package, struct in_addr ip);
struct filenode* getfilenode(char* buf);

void showPackage(struct UDPPackage* package);

#endif
