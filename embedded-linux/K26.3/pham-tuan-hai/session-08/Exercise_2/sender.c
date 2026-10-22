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
	int sender_fd;
	
	//Create sock
	if((sender_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
  	perror("Sock failed");
		exit(1);
		
  }
	
	struct sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	
	//Cau hinh dia chi addr
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(9000);
	dest_addr.sin_addr.s_addr = inet_addr(COLLECTOR_IP);
	
	printf("[Sensor] Target collector: %s:%d\n", COLLECTOR_IP, COLLECTOR_PORT);
	
	for(int i = 0; i < 5; i++) {
  	double load1;
		
		FILE *f1 = fopen("/proc/loadavg", "r");
		if(f1 == NULL) {
			perror("Open f1 failed");
			break;
    }
		
		char line[BUFFER_SIZE];
		fgets(line, sizeof(line), f1);
		sscanf(line, "%lf", &load1);
		double temp = 40 + load1 * 10;
		fclose(f1);
		
		double mem_total = 0, mem_free = 0, mem_used_percent;
		
		FILE *f2 = fopen("/proc/meminfo", "r");
		if(f2 == NULL) {
    	perror("Open f2 failed");
			break;
    }
		
		while(fgets(line, sizeof(line), f2)) {
    	if(strncmp(line, "MemTotal:", 9) == 0) {
				sscanf(line, "MemTotal: %le kB", &mem_total);
      }
			else if (strncmp(line, "MemFree:", 8) == 0) {
      	sscanf(line, "MemFree: %le kB", &mem_free);
      }
    }
		
		mem_used_percent = (double)(mem_total - mem_free) / mem_total * 100.0;
		
		char message[128];
		
		snprintf(message, sizeof(message), "id=sensor-0%d temp=%.1f mem_used=%.1f%%", i + 1, temp, mem_used_percent); 
		
		if(sendto(
    	sender_fd,
			message, strlen(message), 0,
			(struct sockaddr *)&dest_addr,
			sizeof(dest_addr)
    ) < 0) {
    	perror("Sendto failed");
			break;
    }
		
		printf("[Sent %d/5] %s\n",i + 1 ,message);
		sleep(2);
  }
	printf("[Sensor] Done.\n");
	close(sender_fd);
	return 0;
}