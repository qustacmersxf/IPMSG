/*************************************************************************
    > File Name: main.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Thu 10 May 2018 03:20:43 PM CST
 ************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "myinclude.h"
#include "ipmsg.h"
#include "userlist.h"
#include "filelist.h"
#include "udp.h"
#include "tcp.h"
#include "input.h"

#define PORT 2425

int main(int argc, char* argv[]){
	pthread_t tid_lsnUDP;
	pthread_t tid_lsnInput;
	pthread_t tid_lsnTCP;
	struct lsnUDPArg UDParg;
	struct lsnInputArg inputArg;

	UDPInit(&UDParg);
	inputInit();
	TCPInit();
	pthread_create(&tid_lsnUDP, NULL, lsnUDP, (void*)(&UDParg));
	inputArg.tid_lsnUDP = tid_lsnUDP;
	pthread_create(&tid_lsnTCP, NULL, lsnTCP, NULL);
	inputArg.tid_lsnTCP = tid_lsnTCP;
	pthread_create(&tid_lsnInput, NULL, lsnInput, (struct lsnInputArg*)(&inputArg));
	pthread_join(tid_lsnUDP, NULL);
	pthread_join(tid_lsnInput, NULL);

	list_release(ulist);
	filelist_release(flist);

	return 0;
}
