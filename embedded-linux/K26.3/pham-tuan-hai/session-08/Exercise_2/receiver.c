#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define COLLECTOR_IP   "127.0.0.1"
#define COLLECTOR_PORT 9000
#define BUFFER_SIZE    128

int main() {
	int collector_fd;
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));
	
	//Create sock
	if((collector_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
  	perror("Sock failed");
		exit(1);
  }

	int opt = 1;
	if (setsockopt(collector_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
	    perror("setsockopt failed");
	    exit(1);
	}
	
	struct sockaddr_in collector_addr;
	
	memset(&collector_addr, 0, sizeof(collector_addr));
	
	collector_addr.sin_family = AF_INET;
	collector_addr.sin_port = htons(9000);
	collector_addr.sin_addr.s_addr = INADDR_ANY;
	
	printf("[Collector] Listening on %s:%d...\n", COLLECTOR_IP, COLLECTOR_PORT);
	
	if(bind(collector_fd, (struct sockaddr*)&collector_addr, sizeof(collector_addr)) < 0) {
  	perror("Bind failed");
		exit(1);
  }
	
	while(1) {
		struct sockaddr_in sender_addr;
		socklen_t addr_len = sizeof(sender_addr);
		
		int bytes_received = recvfrom(collector_fd,  buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&sender_addr, &addr_len);
		
		if (bytes_received < 0) {
    	perror("Recv failed");
    	continue; // Lỗi thì bỏ qua, chờ gói tin khác
		}
  	
		buffer[bytes_received] = '\0';
		
				/* Generate timestamp */
		time_t now = time(NULL);
		struct tm *t = localtime(&now);
		char ts[16];
		strftime(ts, sizeof(ts), "%H:%M:%S", t);
		
		char *sender_ip = inet_ntoa(sender_addr.sin_addr);
		int sender_port = ntohs(sender_addr.sin_port);
		printf("[%s] %s:%d -> %s\n", ts, sender_ip, sender_port, buffer);
  }
	
	return 0;
}
	