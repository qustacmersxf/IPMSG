/*************************************************************************
    > File Name: userlist.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Wed 09 May 2018 04:56:29 PM CST
 ************************************************************************/

#include "userlist.h"

extern struct userlist* ulist;

int list_init(struct userlist* list){
	list->head = (struct usernode*)malloc(sizeof(struct usernode));
	list->tail = (struct usernode*)malloc(sizeof(struct usernode));
	list->head->pre = list->tail->next = NULL;
	list->head->next = list->tail;
	list->tail->pre = list->head;
	list->p = list->head;
	list->cnt = 0;
	return 0;
}

int list_insert(struct userlist* list, struct usernode* usr){
	usr->num = list->cnt;
	list->p = list->tail->pre;
	usr->next = list->tail;
	usr->pre = list->p;
	list->tail->pre = usr;
	list->p->next = usr;
	list->cnt++;
	return 0;
}

int list_find(struct userlist* list, struct in_addr ip){
	list->p = list->head->next;
	while (list->p != list->tail){
		if (list->p->ip.s_addr == ip.s_addr){
			break;
		}
		list->p = list->p->next;
	}
	if (list->p == list->tail){
		return -1;
	}
	return 0;
}

int list_findbynum(struct userlist* list, int num){
	list->p = list->head->next;
	while (list->p != list->tail){
		if (list->p->num == num){
			break;
		}
		list->p = list->p->next;
	}
	if (list->p == list->tail){
		return -1;
	}
	return 0;
}

int list_delete(struct userlist* list, struct in_addr ip){
	int ret = list_find(list, ip);
	if (ret != -1){
		list->p->pre->next = list->p->next;
		list->p->next->pre = list->p->pre;
		free(list->p);
		//list->cnt--;
	}
	return ret;
}

int list_release(struct userlist* list){
	list->p = list->head;
	while (list->p != list->tail){
		list->p = list->p->next;
		free(list->p->pre);
	}
	free(list->tail);
	return 0;
}

int list_print(struct userlist* list){
	if (list->cnt == 0){
		fprintf(stdout, "用户列表为空\n");
		return 0;
	}

	list->p = list->head->next;
	fprintf(stdout, "%-15s\t%-15s\t%-15s\t%-15s\n", "编号", "用户名", "主机名", "IP");
	while (list->p != list->tail){
		fprintf(stdout, "%-15d\t%-15s\t%-15s\t%-15s\n", 
			list->p->num, list->p->name, list->p->host, inet_ntoa(list->p->ip));
		list->p = list->p->next;
	}
	return 0;
}

