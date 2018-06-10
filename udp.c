/*************************************************************************
    > File Name: udp.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Thu 10 May 2018 04:17:27 PM CST
 ************************************************************************/

#include "udp.h"

struct in_addr ip;
struct in_addr ip_br;
char host[80] = {0};
char name[80] = {0};
char info[1024] = {0};
int len_info = 0;
int lsnSocket = 0;

struct userlist* ulist;

//UDP初始化，返回绑定UPD的socket套接字
int UDPInit(struct lsnUDPArg* arg){
	int set = 1;
	struct ifreq ifr;
	struct sockaddr_in sin;
	char* p;
	char ip_s[15] = {0};
	int i;
	unsigned char macaddr[7] = {0};

	//初始化用户列表
	ulist = (struct userlist*)malloc(sizeof(struct userlist));
	list_init(ulist);
	
	//绑定UDP
	if ((arg->lsnSocket= socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("socket fail");
		return -1;
	}
	setsockopt(arg->lsnSocket, SOL_SOCKET, SO_REUSEADDR|SO_BROADCAST, &set, sizeof(int));
	memset(&(arg->recvAddr), 0, sizeof(struct sockaddr_in));
	arg->recvAddr.sin_family = AF_INET;
	arg->recvAddr.sin_port = htons(2425);
	arg->recvAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(arg->lsnSocket, (struct sockaddr*)&(arg->recvAddr), sizeof(struct sockaddr)) == -1){
		perror("bind fail");
		return -1;
	}
	lsnSocket = arg->lsnSocket;

	//获取本地IP
	strncpy(ifr.ifr_name, "eth0", 16);
	if (ioctl(arg->lsnSocket, SIOCGIFADDR, &ifr) < 0){
		perror("ip ioctl fail");
		return -1;
	}
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	ip = sin.sin_addr;
	sprintf(ip_s, "%s", inet_ntoa(ip));

	//发送广播的IP
	ip_br = ip;
	ip_br.s_addr |= 0xff000000;

	//获取当前用户名和主机名
	struct passwd* pwd = getpwuid(getuid());
	sprintf(name, "%s", pwd->pw_name);
	gethostname(host, sizeof(host));

	//获取MAC地址
	strcpy(ifr.ifr_name, "eth0");
	if (ioctl(arg->lsnSocket, SIOCGIFHWADDR, &ifr) < 0){
		perror("MAC ioctl fail");
		return -1;
	}
	memcpy(macaddr, ifr.ifr_hwaddr.sa_data, 6);
	macaddr[6] = '\0';
	
	p = (char*)info;
	strncpy(p, name, strlen(name)); p = p + strlen(p) + 1;
	strncpy(p, "WorkGroup", 9); p = p + strlen(p) + 1;
	for (i=0; i<6; i++) {
		sprintf(p, "%02x-", macaddr[i]);
		p += 3;
	}
	p--; *p = '\0';
	p += 6;
	strncpy(p, ip_s, strlen(ip_s)); p = p + strlen(p) + 2;
	sprintf(p, "%d", 5); p = p + strlen(p) + 1;
	sprintf(p, "%s", "10000001"); p = p + strlen(p) + 1;
	sprintf(p, "%s", "Hello world"); p = p + strlen(p) + 1;
	len_info = getbuflen(info, 1024);

	return 0;
}

void* lsnUDP(void* arg){
	struct lsnUDPArg* _arg = (struct lsnUDPArg*)arg;
	struct filenode* node;
	int recvBtyes;
	char recvbuf[1024];
	char msg[200];
	int addrLen = sizeof(struct sockaddr_in);
	struct UDPPackage package;
	int cmd, opt;
	char ns[20];
	int i;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	sendUDP(time(NULL), IPMSG_BR_ENTRY, info, len_info, ip_br);
	fprintf(stdout, "[系统消息]: 您已上线，输入'help'查看使用方法\n");

	while (1){
		if ((recvBtyes = recvfrom(_arg->lsnSocket, recvbuf, 1024, 0,
			(struct sockaddr*)&_arg->recvAddr, &addrLen)) != -1){
			recvbuf[recvBtyes] = '\0';
		}
		//for (i=0; i<200; i++) fprintf(stdout, "%c", recvbuf[i]); fprintf(stdout, "  recvbuf end\n");
		memset(&package, 0, sizeof(struct UDPPackage));
		analysis(recvbuf, &package);
		cmd = GET_MODE(package.cmd);
		opt = GET_OPT(package.cmd);
		//fprintf(stdout, "cmd=%d  opt=%d\n", cmd, opt);
		switch(cmd){
			case IPMSG_BR_ENTRY:
				addusr(&package, _arg->recvAddr.sin_addr);
				sendUDP(time(NULL), IPMSG_ANSENTRY, info, len_info, _arg->recvAddr.sin_addr);
				break;
			case IPMSG_BR_EXIT:
   	  			rmusr(_arg->recvAddr.sin_addr);
				break;
			case IPMSG_ANSENTRY:
	   			addusr(&package, _arg->recvAddr.sin_addr); 
				fprintf(stdout, "[系统消息]: 用户%s已在线\n", inet_ntoa(_arg->recvAddr.sin_addr));
				break;
			case IPMSG_SENDMSG:
				if ((opt & IPMSG_SENDCHECKOPT)/* == IPMSG_SENDCHECKOPT*/){
	 				sprintf(ns, "%llu", package.num);
					sendUDP(time(NULL), IPMSG_RECVMSG, ns, strlen(ns), _arg->recvAddr.sin_addr);
					if ((opt & IPMSG_FILEATTACHOPT)/* == IPMSG_FILEATTACHOPT*/){
 						node = getfilenode(package.info);
						node->ip = _arg->recvAddr.sin_addr;
						node->type = FILETYPE_RECV;

						pthread_mutex_lock(&flist_mutex);
						filelist_insert(flist, node);
						pthread_mutex_unlock(&flist_mutex);
						
						fprintf(stdout, "[系统消息]: 你有一个新文件待接收, 请打印文件列表查看\n");
						break;
					}
				}
				displaymsg(&package, _arg->recvAddr.sin_addr);
				break;
			case IPMSG_RECVMSG:
				fprintf(stdout, "[系统消息]: 用户%s已收到信息\n", inet_ntoa(_arg->recvAddr.sin_addr));
				break;
			default:
				break;
		}
		memset(recvbuf, 0, 1024);
	}
}
//添加新用户到用户列表
int addusr(struct UDPPackage* package, struct in_addr ip){	
	struct usernode *node = (struct usernode*)malloc(sizeof(struct usernode));

	if (list_find(ulist, ip) == 0){
		free(node);
		return 0;
	}

	sprintf(node->name, "%s", package->info);
	sprintf(node->host, "%s", package->host);
	node->ip = ip;
	list_insert(ulist, node);
	fprintf(stdout, "[系统消息]: 用户%s已上线\n", inet_ntoa(ip));
	return 0;
}
//从用户列表中移除用户
int rmusr(struct in_addr ip){
	list_delete(ulist, ip);
	fprintf(stdout, "[系统消息]: 用户%s已下线\n", inet_ntoa(ip));
	return 0;
}
//显示用户发来的信息
void displaymsg(struct UDPPackage* package, struct in_addr ip){
	list_find(ulist, ip);
	if (ulist->p == ulist->tail){
		fprintf(stdout, "%s", "[系统消息]: 没有找到该用户\n");
		return;
	}
	fprintf(stdout, "[%s]: %s", inet_ntoa(ulist->p->ip), (char*)package->info);
}

struct filenode* getfilenode(char* buf){
	struct filenode* node = (struct filenode*)malloc(sizeof(struct filenode));
	char* p;

	buf++;
	p = strchr(buf, ':');
	*p = '\0';
	sscanf(buf, "%d", &node->fnum);
	buf = p+1;

	p = strchr(buf, ':');
	*p = '\0';
	sscanf(buf, "%s", node->name);
	buf = p+1;

	p = strchr(buf, ':');
	*p = '\0';
	sscanf(buf, "%x", &node->size);
	buf = p+1;

	buf += 4;

	p = strchr(buf, ':');
	*p = '\0';
	sscanf(buf, "%ld", &node->ctime);
	buf = p+1;

	p = strchr(buf, ':');
	*p = '\0';
	sscanf(buf, "%ld", &node->ltime);
	buf = p+1;

	sscanf(buf, "%ld", &node->atime);

	return node;
}

int getbuflen(char* buf, int size){
	int i = size - 1;
	for (; i>=0; i--){
		if (buf[i] != '\0') break;
	}
	return i+1;
}

//将buf通过UDP发送到地址dstip，若成功，则返回发送字节长度，若失败，则返回-1
int sendUDP(unsigned long long pkgnum, unsigned int cmd, char* info, int len, struct in_addr ip){
	int optval = 1;
	int sendBytes;
	struct UDPPackage package;
	char buf[sizeof(struct UDPPackage) + 10] = {0};
	int i;

	memset(&package, 0, sizeof(struct UDPPackage));
	strcpy(package.version, "1");
	package.num = pkgnum;
	sprintf(package.name, "%s", name); //动态获取用户名
	sprintf(package.host, "%s", host);
	package.cmd = cmd;
	memcpy(package.info, info, len_info);
	makepackage(buf, &package);

	struct sockaddr_in theirAddr;
	memset(&theirAddr, 0, sizeof(struct sockaddr_in));
	theirAddr.sin_family = AF_INET;
	theirAddr.sin_addr = ip;
	theirAddr.sin_port = htons(2425);
	if ((sendBytes = sendto(lsnSocket, buf, getbuflen(buf, sizeof(struct UDPPackage) + 10), 0, (struct sockaddr*)&theirAddr, sizeof(struct sockaddr))) == -1){
		perror("sendto fail in sendUDP");
		return -1;
	}

	return sendBytes;
}

//解析协议buf，结果放在package里
void analysis(char* buf, struct UDPPackage* package){
	char* pos;
	char* tmp = buf;
	
	pos = strchr(buf, ':'); 
	*pos = '\0';
	sprintf(package->version, "%s", buf);
	buf = pos+1;
	
	pos = strchr(buf, ':'); 
	*pos = '\0';
	package->num = strtoull(buf, NULL, 10);
	buf = pos+1;

	pos = strchr(buf, ':');
	*pos = '\0';
	sprintf(package->name, "%s", buf);
	buf = pos+1;

	pos = strchr(buf, ':');
	*pos = '\0';
	sprintf(package->host, "%s", buf);
	buf = pos+1;

	pos = strchr(buf, ':');
	*pos = '\0';
	package->cmd = strtoull(buf, NULL, 10);
	buf = pos+1;

	memcpy(package->info, buf, getbuflen(buf, 1024-(buf-tmp)));
}
//将存有需要发送信息的package组合成符合IPMSG协议的buf
void makepackage(char* buf, struct UDPPackage* package){
	memset(buf, 0, sizeof(struct UDPPackage) + 10);
	sprintf(buf, "%s:", package->version);
	sprintf(buf+strlen(buf), "%llu:", package->num);
	sprintf(buf+strlen(buf), "%s:", package->name);
	sprintf(buf+strlen(buf), "%s:", package->host);
	sprintf(buf+strlen(buf), "%llu:", package->cmd);
	if (*(package->info + strlen(package->info) + 1) != '\0'){
		memcpy(buf+strlen(buf), package->info, len_info);
	}else{
		sprintf(buf+strlen(buf), "%s", package->info);
	}
}


void showPackage(struct UDPPackage* package){
	fprintf(stdout, "showPackage:  %s %llu %s %s %llu %s\n", 
			package->version, package->num, package->name,
			package->host, package->cmd, package->info);
}
