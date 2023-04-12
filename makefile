all: server client

server: functions.c server.c
	gcc server.c -o server

client: functions.c client.c
	gcc client.c -o client

clean:
	rm -fr server client