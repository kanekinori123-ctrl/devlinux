CFLAGS = -Wall -Wextra -Werror -g

all: client server

client: client.c
	gcc $(CFLAGS) client.c -o client
	
server: server.c
	gcc $(CFLAGS) server.c -o server
	
clean:
	rm -rf server client