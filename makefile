all: server client ctest
# all: server client server_file_manager

server: functions.c  server.c functions.h
	gcc server.c -o server

client: functions.c client.c functions.h
	gcc client.c -o client

ctest: functions.c client_test.c functions.h
	gcc -g client_test.c -o ctest

# server_file_manager: functions.c server_file_manager.c
# 	gcc -o server_file_manager server_file_manager.c

clean:
	rm -fr server client