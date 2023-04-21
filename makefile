all: server client ctest fget thread
# all: server client server_file_manager

server: functions.c  server.c functions.h
	gcc -g server.c -o server

client: functions.c client.c functions.h
	gcc -g client.c -o client

fget: functions.c fget.c functions.h
	gcc -g fget.c -o fget

ctest: functions.c client_test.c functions.h
	gcc -g client_test.c -o ctest

thread: functions.c test_multithreads.c functions.h
	gcc -g test_multithreads.c -o thread

clean:
	rm -fr server client fget
