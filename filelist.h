/*************************************************************************
    > File Name: userlist.h
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Thu 10 May 2018 04:21:57 PM CST
 ************************************************************************/

#ifndef FILELIST_H
#define FILELIST_H

#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

#define FILETYPE_SEND 0
#define FILETYPE_RECV 1

struct filenode{
	int type;
	char name[30];
	char path[100];
	int lnum;
	int fnum;
	int pkgnum;
	unsigned int size;
	time_t ctime;
	time_t ltime;
	time_t atime;
	struct in_addr ip;
	struct filenode *next, *pre;
};
struct filelist{
	struct filenode* head;
	struct filenode* tail;
	int cnt;
	struct filenode* p;
};

extern struct filelist* flist;
extern pthread_mutex_t flist_mutex;

int filelist_init(struct filelist* list);
int filelist_insert(struct filelist* list, struct filenode* usr);
int filelist_find(struct filelist* list, int num);
int filelist_findbyfnum(struct filelist* list, int fnum);
int filelist_delete(struct filelist* list, int num);
int filelist_release(struct filelist* list);
int filelist_print(struct filelist* list);
int file_getinfo(char* pathname, struct filenode* node);

#endif
