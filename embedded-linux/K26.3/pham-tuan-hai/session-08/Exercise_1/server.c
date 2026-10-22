#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#define SOCKET_PATH "/tmp/monitor.sock"
#define BUFFER_SIZE 256

int server_fd = -1, client_fd = -1;

void handle_sigint(int sig) {
	(void)sig;
	if (client_fd != -1) {
	  close(client_fd);
	}
  // Đóng cửa chính của Server
  if (server_fd != -1) {
	  close(server_fd);
  }
	
	unlink(SOCKET_PATH);
	exit(0);
}

int main() {
	struct sockaddr_un addr;
	signal(SIGINT, handle_sigint);
	char buffer[256];
	
	// 1. Don dep, xoa file socket cu~ neu chuong trinh truoc bi crash
	unlink(SOCKET_PATH);
	
	// 2. Create Socket ( AF_UNIX cho local, SOCK_STREAM cho reliable data
	if((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
  	perror("Socket failed");
		exit(1);
  }
	
	// 3. Cau hinh dia chi
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
	
	// 4. Bind (Gan socket voi file) va Listen
	if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
  	perror("Bind failed");
		exit(1);
  }

	if(listen(server_fd, 5) < 0) {
  	perror("Listen failed");
		exit(1);
  }
	
	printf("[Server] Dang cho khach hang tai %s...\n", SOCKET_PATH);
	while(1) {
		
		if((client_fd = accept(server_fd, NULL, NULL)) < 0) {
			perror("Accept failed");
			continue; // Lỗi đón khách này thì bỏ qua, chờ khách khác (không nên exit)
		}
		
		printf("[Server] Customer founded!\n");
		
		while(1) {
			
			// 6. Recv order
			memset(buffer, 0, sizeof(buffer));
			int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
			if(bytes_received < 0) {
      	perror("Recv failed");
				break;
      } else if (bytes_received == 0) {
      	break;
      }
			
			buffer[strcspn(buffer, "\n")] = 0;
			
			if(strcmp(buffer, "cpu") == 0) {
	    	printf("[Daemon] CMD: %s\n", buffer);
				double load1;
				FILE *f = fopen("/proc/loadavg", "r");
				if (f == NULL) {
	      	break;
	      }else {
					char line[128];
	      	if (fgets(line, sizeof(line), f)) {
		        // 2. Dùng sscanf bốc ngay con số thập phân đầu tiên ra
		        sscanf(line, "%lf", &load1);
					}
					fclose(f);
					char response[BUFFER_SIZE];
					snprintf(response, sizeof(response), "load_avg=%lf", load1);
					
					// 7. Responding back the message
					if(send(client_fd, response, strlen(response), 0) < 0) {
          	perror("Send failed");
						exit(1);
          }
	      }
	    }
			else if (strcmp(buffer, "mem") == 0) {
	    	printf("[Daemon] CMD: %s\n", buffer);
				/* Read mem_total and mem_free from /proc/meminfo */
				long mem_total = 0; 
				long mem_free = 0; 
				char line[128];
				FILE *f = fopen("/proc/meminfo", "r");
				if (f == NULL) {
	      	perror("Open file failed");
					break;
	      }else {
			    while (fgets(line, sizeof(line), f)) {
						/* look for "MemTotal:" and "MemFree:" lines */
						if(strncmp(line, "MemTotal:", 9) == 0) {
							sscanf(line, "MemTotal: %ld kB", &mem_total);
	          }
						else if (strncmp(line, "MemFree:", 8) == 0) {
	          	sscanf(line, "MemFree: %ld kB", &mem_free);
	          }
						
						if (mem_total > 0 && mem_free > 0) {break;}
					}
					fclose(f);
					
					char response[BUFFER_SIZE];
					snprintf(response, sizeof(response), "mem_total=%ld kB mem_free=%ld kB", mem_total, mem_free);
					
					// 7. Responding back the message
					if(send(client_fd, response, strlen(response), 0) < 0){
						perror("Send failed");
						exit(1);
          }
	      }
			} else if (strcmp(buffer, "quit") == 0) {
				break;
			} else {
				char *err_msg = "Unknown command\n";
				send(client_fd, err_msg, strlen(err_msg), 0);
			}
		}
		printf("[Daemon] Client disconnected. Waiting for next client...\n");
		close(client_fd);
		client_fd = -1;
  }
	
	// 8. closing
	close(server_fd);
	unlink(SOCKET_PATH);
	return 0;
}