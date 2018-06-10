/*************************************************************************
    > File Name: userlist.h
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Thu 10 May 2018 04:21:57 PM CST
 ************************************************************************/

#ifndef USERLIST_H
#define USERLIST_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

struct usernode{
	struct in_addr ip;
	char name[80];
	char host[80];
	int num;
	struct usernode *next, *pre;
};
struct userlist{
	struct usernode* head;
	struct usernode* tail;
	int cnt;
	struct usernode* p;
};

extern struct userlist* ulist;

int list_init(struct userlist* list);
int list_insert(struct userlist* list, struct usernode* usr);
int list_delete(struct userlist* list, struct in_addr ip);
int list_find(struct userlist* list, struct in_addr ip);
int list_findbynum(struct userlist* list, int num);
int list_release(struct userlist* list);
int list_print(struct userlist* list);

#endif
