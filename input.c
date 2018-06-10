/*************************************************************************
    > File Name: input.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Wed 23 May 2018 09:32:26 AM CST
 ************************************************************************/

#include "input.h"

void inputInit(){
	flist = (struct filelist*)malloc(sizeof(struct filelist));
	filelist_init(flist);
}

void* lsnInput(void* arg){
	char cmd[100];
	struct lsnInputArg* inputArg = (struct lsnInputArg*)arg;
	while (1){
		memset(cmd, 0, 100);
		fscanf(stdin, "%s", cmd);
		if (strcmp(cmd, "help") == 0){
			showhelp();
		}else if (strcmp(cmd, "ls") == 0){
			fscanf(stdin, "%s", cmd);
			if (strcmp(cmd, "usrs") == 0){
				list_print(ulist);
			}else if (strcmp(cmd, "file") == 0){
				filelist_print(flist);
			}
		}else if (strcmp(cmd, "sendto") == 0){
			handleSendTo();
		}else if (strcmp(cmd, "sendfile") == 0){
			handleSendfile();
		}else if (strcmp(cmd, "getfile") == 0){
			handleGetfile();
		}else if (strcmp(cmd, "exit") == 0){
			handleExit();
			pthread_cancel(inputArg->tid_lsnUDP);
			pthread_cancel(inputArg->tid_lsnTCP);
			pthread_exit(NULL);
		}else{
			fprintf(stdout, "输入命令错误！\n");
			setbuf(stdin, NULL);
		}
	}
}

void showhelp(){
	fprintf(stdout, "################################################################################\n");
	fprintf(stdout, "## 1.help                    显示帮助                                         ##\n");
	fprintf(stdout, "## 2.ls usrs/rvfile          显示用户/文件列表                                ##\n");
	fprintf(stdout, "## 3.sendto num msg          向编号为num的用户发送信息msg                     ##\n");
	fprintf(stdout, "## 4.sendfile num pathname   向编号为num的用户发送路径为pathname的文件        ##\n");
	fprintf(stdout, "## 5.getfile num             接收待接收文件列表中编号为num的文件              ##\n");
	fprintf(stdout, "## 6.exit                    退出程序                                         ##\n");
	fprintf(stdout, "################################################################################\n");
}

void handleSendTo(){
	int num;
	char msg[1024];
	struct in_addr dstip;

	fscanf(stdin, "%d", &num);
	list_findbynum(ulist, num);
	dstip = ulist->p->ip;
	memset(msg, 0, 1024);
	getchar();
	fgets(msg, 1024, stdin);

	sendUDP(time(NULL), IPMSG_SENDMSG|IPMSG_SENDCHECKOPT, msg, getbuflen(msg, 1024), dstip);
}

void handleSendfile(){
	struct in_addr dstip;
	struct filenode* node;
	int num;
	char aip[20] = {0};
	char pathname[80] = {0};
	char buf[200] = {0};
	char* p;
	int fd;

	fscanf(stdin, "%d", &num);
	list_findbynum(ulist, num);
	dstip = ulist->p->ip;
	getchar();
	fscanf(stdin, "%s", pathname);
	if ((fd = open(pathname, O_RDONLY)) < 0){
		perror("handleSendfile() File open error");
		return ;
	}
	close(fd);
	node = (struct filenode*)malloc(sizeof(struct filenode));
	file_getinfo(pathname, node);
	node->type = FILETYPE_SEND;
	node->pkgnum = time(NULL);
	node->fnum = node->pkgnum - 1;
	node->ip = dstip;

	pthread_mutex_lock(&flist_mutex);
	filelist_insert(flist, node);
	pthread_mutex_unlock(&flist_mutex);
	
	p = (char*)buf + 1;
	sprintf(p, "%d:", node->fnum);
	sprintf(p+strlen(p), "%s:", node->name);
	sprintf(p+strlen(p), "%x:0:1:", node->size);
	sprintf(p+strlen(p), "%ld:", (long)node->ctime);
	sprintf(p+strlen(p), "%ld:", (long)node->ltime);
	sprintf(p+strlen(p), "%ld:", (long)node->atime);
	sendUDP(time(NULL), IPMSG_SENDMSG|IPMSG_SENDCHECKOPT|IPMSG_FILEATTACHOPT, buf, strlen(p)+2, dstip);
}

void handleGetfile(){
	int num;
	pthread_t tid;

	fscanf(stdin, "%d", &num);

	pthread_create(&tid, NULL, recvfile, (void*)&num);
}

void handleExit(){
	ulist->p = ulist->head->next;
	while (ulist->p != ulist->tail){
		sendUDP(time(NULL), IPMSG_BR_EXIT, name, strlen(name), ulist->p->ip);
		ulist->p = ulist->p->next;
	}
}