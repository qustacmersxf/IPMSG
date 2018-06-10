/*************************************************************************
    > File Name: tcp.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Sun 27 May 2018 07:37:19 AM CST
 ************************************************************************/

#include "tcp.h"

int tcpsockfd;

void TCPInit(){
	struct sockaddr_in servaddr;

	if ((tcpsockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("TCP socket error");
		return ;
	}
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	if (bind(tcpsockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
		perror("TCP bind error");
		return;
	}
	listen(tcpsockfd, 5);
}

void* lsnTCP(void* arg){
	int client;
	socklen_t client_len;
	pthread_t tid;
	struct sockaddr_in clientaddr;

	client_len = sizeof(clientaddr);
	while (1){
		fprintf(stdout, "TCP listening...\n");
		client = accept(tcpsockfd, (struct sockaddr*)&clientaddr, &client_len);
		fprintf(stdout, "TCP accepted...\n");
		pthread_create(&tid, NULL, sendfile, (void*)&client);
	}
}

void* sendfile(void* arg){
	int readlen;
	int fd;
	char buf[1024];
	char pathname[100];
	int client = *((int*)arg);
	struct UDPPackage package;
	int fnum, pkgnum, offset;
	int i;

	memset(buf, 0, 1024);
	readlen = read(client, buf, 1024);

	//for (i=0; i<200; i++) fprintf(stdout, "%c", buf[i]); fprintf(stdout, " TCP recvbuf end\n");
	analysis(buf, &package);

	parsenum(package.info, &fnum, &pkgnum, &offset);
	filelist_findbyfnum(flist, fnum);
	memset(pathname, 0, 100);
	sprintf(pathname, "%s/%s", flist->p->path, flist->p->name);
	if ((fd = open(pathname, O_RDONLY)) < 0){
		perror("lsnTCP() open file error");
		close(client);
		return;
	}
	memset(buf, 0, 1024);
	fprintf(stdout, "Send file...\n");
	while((readlen = read(fd, buf, 1024)) > 0){
		write(client, buf, readlen);
	}
	fprintf(stdout, "Send file finished\n");
	close(client);
	close(fd);

	pthread_mutex_lock(&flist_mutex);
	filelist_delete(flist, flist->p->lnum);	
	fprintf(stdout, "filelist_delete end\n");
	pthread_mutex_unlock(&flist_mutex);
}

void* recvfile(void* arg){
	int num = *((int*)arg);
	int client;
	int fd;
	int recvlen;
	char buf[sizeof(struct UDPPackage) + 10];
	struct sockaddr_in servaddr;
	struct UDPPackage package;
	pthread_t tid;

	fprintf(stdout, "recvfile()\n");
	
	if(filelist_find(flist, num) < 0){
		fprintf(stdout, "The file is not exist!\n");
		return (void*)-1;
	}
	memset(&package, 0, sizeof(struct UDPPackage));
	memset(buf, 0, sizeof(buf));
	strcpy(package.version, "1");
	package.num = time(NULL);
	sprintf(package.name, "%s", name); //动态获取用户名
	sprintf(package.host, "%s", host);
	package.cmd = IPMSG_GETFILEDATA;
	sprintf(package.info, "%d:%d:%d", flist->p->pkgnum, flist->p->fnum, 0);
	makepackage(buf, &package);

	client = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr = flist->p->ip;
 	servaddr.sin_port = htons(SERV_PORT);
 	if (connect(client, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
 		perror("connet error");
 		return (void*)-1;
	}
	fprintf(stdout, "connect successed\n");
	write(client, buf, getbuflen(buf, sizeof(buf)));

	if ((fd = open(flist->p->name, O_WRONLY|O_CREAT,
			  S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0){
		perror("recvfile() File open error");
		return (void*)-1;
	}

	fprintf(stdout, "Recevie file...\n");
	memset(buf, 0, 1024);
	while ((recvlen = read(client, buf, 1024)) > 0){
		write(fd, buf, recvlen);
	}
	fprintf(stdout, "Recevie file finisded.\n");
	close(fd);
	close(client);
	
	pthread_mutex_lock(&flist_mutex);
	filelist_delete(flist, num);
	pthread_mutex_unlock(&flist_mutex);

	return (void*)0;
}

void parsenum(char* buf, int* fnum, int* pkgnum, int* offset){
	char* p = strchr(buf, ':');
	*p = '\0';
	sscanf(buf, "%d", pkgnum);
	buf = p+1;

	p = strchr(buf, ':');
	*p = '\0';
	sscanf(buf, "%d", fnum);
	buf = p+1;

	sscanf(buf, "%d", offset);
}
