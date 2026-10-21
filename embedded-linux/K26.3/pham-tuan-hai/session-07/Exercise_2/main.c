#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

void handle_sigusr1(int sig);

int main() {
	sigset_t block_set;
	
	signal(SIGUSR1, handle_sigusr1);
	
	pid_t pid = fork();
	
	if(pid < 0) {
  	perror("Fork failed");
		exit(1);
  }
	
	else if(pid == 0) {
		// ==========================================
    // KHU VỰC CỦA TIẾN TRÌNH CON (WORKER)
    // ==========================================
		sleep(2);
		
		kill(getppid(), SIGUSR1);
		printf("[WORKER] Sent READY signal to gateway\n");
		
		exit(7);
  }
	
	else if (pid > 0) {
  	printf("[GATEWAY] Worker PID = %d\n", pid);
		
		sigemptyset(&block_set);
		sigaddset(&block_set, SIGUSR1);
		
		sigprocmask(SIG_BLOCK, &block_set, NULL);
		
		sleep(5);
		
		sigprocmask(SIG_UNBLOCK, &block_set, NULL);
		
		int status;
		wait(&status);
		if (WIFEXITED(status)) {
    	printf("[GATEWAY] Worker exited with code %d\n", WEXITSTATUS(status));
    }
		
		
	}
	return 0;
}

void handle_sigusr1(int sig) {
	(void)sig;
	
	printf("[GATEWAY] Worker reported READY signal received\n");
	fflush(stdout);
	return;
}