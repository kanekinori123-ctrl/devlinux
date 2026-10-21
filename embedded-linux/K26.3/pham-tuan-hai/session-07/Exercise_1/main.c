#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t reading_count = 0;

void handle_sigint(int sig);
void handle_sigterm(int sig);
void handle_sigusr1(int sig);

int main() {
	signal(SIGINT, handle_sigint);
  signal(SIGTERM, handle_sigterm);
  signal(SIGUSR1, handle_sigusr1);

	while(1){
		reading_count++;
  	printf("[INFO] Sensor reading #%d: temperature=10\n", reading_count);
		sleep(1);
  }

	return 0;
}

void handle_sigint(int sig) {
	(void)sig;
	printf("[WARN] Received SIGINT, ignoring...\n");
	return;
}

void handle_sigterm(int sig) {
	(void)sig;
	printf("[INFO] Received SIGTERM, shutting down gracefully...\n");
	exit(0);
}

void handle_sigusr1(int sig) {
	(void)sig;
	printf("[REPORT] Total readings so far: %d\n", reading_count);
	fflush(stdout);
	return;
}