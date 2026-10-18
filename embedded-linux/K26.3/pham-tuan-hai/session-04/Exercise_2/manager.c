#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

extern char environ;

int main() {
while(1) {
printf("=\n");
printf("STUDENT LOOKUP SYSTEM — MANAGER\n");
printf("fork + execve | file: students.txt\n");
printf("=\n");

	printf("[MANAGER] PID: %d\n", getpid());
	printf("Enter student ID ('quit' to exit).");
	
	char student_id[20];
	
	// Xu ly retry neu stdin bi gian doan (EINTR) boi tin hieu
	while(1) {
		if (fgets(student_id, sizeof(student_id), stdin) != NULL) {
			break;
		}
		if (errno == EINTR) {
			clearerr(stdin);
			continue;
		}
		// Neu an Ctrl+D (EOF) hoac loi that su thi thoat
		printf("\n[MANAGER] Exiting. Goodbye!\n");
		return 0;
	}
	
	student_id[strcspn(student_id, "\n")] = 0;
	
	if (strcmp(student_id, "quit") == 0) {
		printf("[MANAGER] Exiting. Goodbye!\n");
		return 0;
	}
	
	pid_t pid = fork();
	
	if (pid == 0) {
		char *args[] = {
			"./searcher",
			student_id,
			"./students.txt",
			NULL
		};
		
		// Thuc thi truyen sang file searcher
		execve("./searcher", args, environ);
		
		// Neu execve that bai (vi du khong thay file ./searcher) moi chay xuong day
		perror("Loi: Khong the thuc thi execve");
		exit(2);
	}
	else if (pid < 0) {
		perror("fork failed");
		exit(1);
	}
	else if (pid > 0) {
		int status;
		printf("[MANAGER] fork() → child PID: %d\n", pid);
		printf("[MANAGER] Waiting for child (waitpid)...\n\n");
		
		waitpid(pid, &status, 0);
		
		// Nhanh 1: Con tu nguyen ket thuc bang lenh exit() hoac return
		if(WIFEXITED(status)) {
			int exit_code = WEXITSTATUS(status);
			
			if(exit_code == 0) {
				printf("[MANAGER] Child (PID %d) exited. code=%d → Found\n", pid, exit_code);
			} else if(exit_code == 1) {
				printf("[MANAGER] Child (PID %d) exited. code=%d → Not found\n", pid, exit_code);
			} else if(exit_code == 2) {
				printf("Loi thuc thi file hoac khong the mo duoc file du lieu\n");
			}
		} 
		// Nhanh 2: Con bi giet boi Signal (Vi du: Crash, kill -9...)
		else if (WIFSIGNALED(status)) {
			int sig = WTERMSIG(status);
			printf("[MANAGER] Child (PID %d) was killed by signal %d\n", pid, sig);
		}
	}
	printf("----------------------------------\n\n");
}

return 0;


}