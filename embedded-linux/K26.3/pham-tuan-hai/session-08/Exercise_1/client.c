#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>

#define SOCKET_PATH "/tmp/monitor.sock"
#define BUFFER_SIZE 256

int main() {
	int client_fd;
	struct sockaddr_un addr;
	char buffer[BUFFER_SIZE];
	
	// 1. Tạo socket
	if((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
  	perror("Socket failed");
		exit(1);
  }
	
	// 2. Cấu hình địa chỉ 
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
	
	// 3. Connect
	if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
  	perror("Server not found");
		exit(1);
  }
	printf("[monitor-cli] Connected to /tmp/monitor.sock\n");
	
	// 4. Sending data
	while(1) {
		printf("> ");
		
		fflush(stdout);
		
		char line[128];
		
		if (fgets(line, sizeof(line), stdin) == NULL) {
    	printf("\nExiting...\n");
			break;
    } 
		
		line[strcspn(line, "\n")] = 0;

    if (strlen(line) == 0) {
        continue; 
    }
		
		if(strcmp(line, "quit") == 0) {
    	break;
    }

		if(send(client_fd, line, strlen(line), 0) < 0) {
			perror("Send failed");
			exit(1);
    }
		
		//5. Waiting for reiceived
		memset(buffer, 0, sizeof(buffer));
		int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
		
		if (bytes_received > 0) {
    	printf("%s\n", buffer);
    } else if (bytes_received == 0) {
    	printf("Server closed\n");
			break;
    } else {
    	perror("Recv error");
			break;
    }
  }
	
	//6. Closing client
	close(client_fd);
	return 0;
}