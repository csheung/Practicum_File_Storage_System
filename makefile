all: server client ctest fget
# all: server client server_file_manager

server: functions.c  server.c functions.h
	gcc server.c -o server

client: functions.c client.c functions.h
	gcc client.c -o client

fget: functions.c fget.c functions.h
	gcc fget.c -o fget

ctest: functions.c client_test.c functions.h
	gcc -g client_test.c -o ctest

clean:
	rm -fr server client fget
