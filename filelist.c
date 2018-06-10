
#include "filelist.h"

struct filelist* flist;
pthread_mutex_t flist_mutex = PTHREAD_MUTEX_INITIALIZER;

int filelist_init(struct filelist* list){
	list->head = (struct filenode*)malloc(sizeof(struct filenode));
	list->tail = (struct filenode*)malloc(sizeof(struct filenode));
	list->head->pre = list->tail->next = NULL;
	list->head->next = list->tail;
	list->tail->pre = list->head;
	list->p = list->head;
	list->cnt = 0;
	return 0;
};

int filelist_insert(struct filelist* list, struct filenode* usr){
	usr->lnum = list->cnt;
	list->p = list->tail->pre;
	usr->next = list->tail;
	usr->pre = list->p;
	list->tail->pre = usr;
	list->p->next = usr;
	list->cnt++;
	return 0;
}

int filelist_find(struct filelist* list, int num){
	list->p = list->head->next;
	while (list->p != list->tail){
		if (list->p->lnum == num){
			break;
		}
		list->p = list->p->next;
	}
	if (list->p == list->tail){
		return -1;
	}
	return 0;
}

int filelist_findbyfnum(struct filelist* list, int fnum){
	list->p = list->head->next;
	while (list->p != list->tail){
		if (list->p->fnum == fnum){
			break;
		}
		list->p = list->p->next;
	}
	if (list->p == list->tail){
		return -1;
	}
	return 0;
}

int filelist_delete(struct filelist* list, int num){
	int ret = filelist_find(list, num);
	fprintf(stdout, "filelist_delete 1\n");
	if (ret != -1){
		fprintf(stdout, "filelist_delete 2\n");
		list->p->pre->next = list->p->next;
		list->p->next->pre = list->p->pre;
		free(list->p);
		//list->cnt--;
	}
	return ret;
}

int filelist_release(struct filelist* list){
	list->p = list->head;
	while (list->p != list->tail){
		list->p = list->p->next;
		free(list->p->pre);
	}
	free(list->tail);
	return 0;
}

int filelist_print(struct filelist* list){
	if (list->cnt == 0){
		fprintf(stdout, "[系统消息]: 没有待处理文件\n");
		return 0;
	}

	list->p = list->head->next;
	fprintf(stdout, "%-15s\t%-15s\t%-15s\t%-15s\n", "编号", "文件名", "大小", "来源IP");
	while (list->p != list->tail){
		fprintf(stdout, "%-15d\t%-15s\t%-15d\t%-15s\n", 
			list->p->lnum, list->p->name, list->p->size, inet_ntoa(list->p->ip));
		list->p = list->p->next;
	}
	return 0;
}

int file_getinfo(char* pathname, struct filenode* node){
	struct stat st;
	int i, plen;
	if (stat(pathname, &st) < 0){
		perror("Get file info error");
		return -1;
	}
	
	plen = strlen(pathname);
	for (i=plen; i>0; i--)
		if (pathname[i] == '/') break;
	memcpy(node->name, pathname+i+1, plen-i);
	memcpy(node->path, pathname, i);
	node->size = st.st_size;
	node->atime = st.st_atime;
	node->ltime = st.st_mtime;
	node->ctime = st.st_ctime;
	return 0;
}
