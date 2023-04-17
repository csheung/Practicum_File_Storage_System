all: server client
# all: server client server_file_manager

server: functions.c  server.c
	gcc server.c -o server

client: functions.c client.c
	gcc client.c -o client

# server_file_manager: functions.c server_file_manager.c
# 	gcc -o server_file_manager server_file_manager.c

clean:
	rm -fr server client 