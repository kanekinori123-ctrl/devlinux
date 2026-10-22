CFLAGS = -Wall -Wextra -Werror -g

all: sender receiver

sender: sender.c
	gcc $(CFLAGS) -o sender sender.c
	
receiver: receiver.c
	gcc $(CFLAGS) -o receiver receiver.c
  
clean:
	rm -rf sender receiver