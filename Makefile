PHONY.:all client server clean
all: server client selserver polserver
server:
	gcc -g -o server server.c
selserver:
	gcc -std=gnu99  -g -o selserver selectServer.c
polserver:
	gcc -std=gnu99 -g -o polserver pollServer.c util.c util.h
client:
	gcc -g -o client client.c
clean:
	rm -f server client selserver polserver
