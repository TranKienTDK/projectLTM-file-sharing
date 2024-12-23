all:
	clear
	gcc -o client client.c -pthread
	gcc -o server server.c linked_list.h -pthread
s:
	clear
	./server 8080
c:
	clear
	./client 127.0.0.1 8080
clean:
	clear
	rm -fr server
	rm -fr client