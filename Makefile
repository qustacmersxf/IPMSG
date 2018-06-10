ipmsg:main.o userlist.o filelist.o udp.o tcp.o input.o
	gcc -g main.o userlist.o filelist.o udp.o tcp.o input.o -o ipmsg -lpthread
main.o:main.c myinclude.h ipmsg.h userlist.h filelist.h udp.h tcp.h input.h
	gcc -g -c main.c -o main.o
userlist.o:userlist.c userlist.h
	gcc -g -c userlist.c -o userlist.o
filelist.o:filelist.c filelist.h
	gcc -g -c filelist.c -o filelist.o -lpthread
udp.o:udp.c udp.h myinclude.h ipmsg.h 
	gcc -g -c udp.c -o udp.o
tcp.o:tcp.c tcp.h udp.h filelist.h
	gcc -g -c tcp.c -o tcp.o -lpthread
input.o:input.c input.h userlist.h filelist.h udp.h tcp.h
	gcc -g -c input.c -o input.o
clean:
	rm *.o ipmsg

re:
	make clean
	make
	./ipmsg
