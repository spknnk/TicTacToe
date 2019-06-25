client:
	gcc -std=c99 -Wall -D_POSIX_SOURCE=600 -D_XOPEN_SOURCE=600 client.c arg.h -o client -lpthread -L/usr/local/lib -lzmq
server:
	gcc -std=c99 -Wall -D_POSIX_SOURCE=600 server.c arg.h -o server -L/usr/local/lib -lzmq
clear:
	rm server client
