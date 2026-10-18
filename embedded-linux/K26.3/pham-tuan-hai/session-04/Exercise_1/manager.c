#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <locale.h>
#define NUM_ORDERS 3

typedef struct {
	int id;
	char name[50];
	int quantity;
	float unit_price;
} Order;

Order orders[NUM_ORDERS] = {
	{1, "Backpack", 2, 350000},
	{2, "Shoes",    1, 500000},
	{3, "Hat",      3, 120000}
};

// Dummy handler: De bat SIGCHLD nhung khong tranh chap viec reap voi waitpid()
void sigchld_handler(int sig) {
	// Su dung tham so 'sig' de compiler khong phan nan -Wunused-parameter
	if (sig == SIGCHLD) {
		// Khong goi waitpid o day, de danh nhiem vu cho vong lap trong main() tinh doanh thu
	}
}

void process_order(Order o) {
	float total = o.quantity * o.unit_price;
	printf("[CHILD-%d] PID: %d | PPID: %d\n", o.id, getpid(), getppid());
	printf("[CHILD-%d] %s x%d — Total: %.0f VND\n",
	       o.id, o.name, o.quantity, total);
	printf("[CHILD-%d] Processing... (sleep 2s)\n\n", o.id);
	// Xu ly EINTR cho sleep: tiep tuc ngu neu bi danh thuc giua chung
	struct timespec req = {2, 0}; // 2 giay, 0 nanogiay
	struct timespec rem;
	while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
		req = rem; // Cap nhat lai thoi gian can ngu bu neu bi ngat
	}
}

int main() {
	setlocale(LC_NUMERIC, "");
	
	//Bat tin hieu SIGCHLD
	struct sigaction sa;
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction failed");
		exit(1);
	}
	
	pid_t pids[NUM_ORDERS];
	int status;
	float total_revenue = 0;
	
	printf("===================================================\n");
	printf("   ORDER PROCESSING SYSTEM — MANAGER (fork+wait)\n");
	printf("===================================================\n");
	
	printf("[MANAGER] PID: %d — spawning %d child processes...\n\n", getpid(), NUM_ORDERS);
	
	printf("--- [child output order may interleave — this is normal] ---\n\n");
	
	//Create sub processes to do process_order
	for(int i = 0; i < NUM_ORDERS; i++) {
	/* fflush before fork to prevent duplicate buffered output */
		fflush(stdout);
		fflush(stderr);
		
		pid_t pid = fork();
		
		if(pid == 0) {
			process_order(orders[i]);
			exit(0);
		} 
		else if (pid > 0){
			pids[i] = pid;
			printf("[MANAGER] fork() order #%d → child PID: %d\n", i + 1, pid);
		} 
		else if (pid < 0) {
			perror("fork failed");
			// Cleanup: Chờ các tiến trình con đã sinh ra trước đó
			for (int j = 0; j < i; j++) {
				waitpid(pids[j], NULL, 0); 
			}
			exit(1);
		}
	}
	
	printf("[MANAGER] All %d children spawned. Starting waitpid()...\n\n", NUM_ORDERS);
	printf("--- [~2 seconds later, all 3 children call exit(0)] ---\n\n");
	
	//Check sub process value and calculate total revenue
	int success_count = 0;
	
	for(int i = 0; i < NUM_ORDERS; i++) {
		waitpid(pids[i], &status, 0);
		
		if(WIFEXITED(status)){
			int exit_code = WEXITSTATUS(status);
			
			printf("[MANAGER] waitpid(%d) — order #%d: exit code=%d → SUCCESS\n", 
			       pids[i], i + 1, exit_code);
		
			if(exit_code == 0){
				success_count++;
				total_revenue += orders[i].quantity * orders[i].unit_price;
			}
		}
	}
	
	//Print summary table
	printf("================= SUMMARY =================\n");
	printf("Total order: %d\n", NUM_ORDERS);
	printf("Success: %d\n", success_count);
	printf("Total revenue: %'.0f\n", total_revenue);
	printf("===========================================\n");
	
	return 0;
}
