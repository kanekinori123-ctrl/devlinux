#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <locale.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

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

void sigchld_handler(int sig) {
if (sig == SIGCHLD) {
// Handler rong de danh thuc cac tien trinh khoi trang thai block
}
}

void process_order(Order o) {
float total = o.quantity * o.unit_price;
printf("[CHILD-%d] PID: %d | PPID: %d\n", o.id, getpid(), getppid());
printf("[CHILD-%d] %s x%d — Total: %.0f VND\n",
o.id, o.name, o.quantity, total);
printf("[CHILD-%d] Processing... (sleep 2s)\n\n", o.id);

// Dung nanosleep de xu ly EINTR mot cach chuan xac
struct timespec req = {2, 0};
struct timespec rem;
while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
	req = rem;
}


}

int main() {
setlocale(LC_NUMERIC, "");

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

// Tao tien trinh con
for(int i = 0; i < NUM_ORDERS; i++) {
	fflush(stdout);
	fflush(stderr);
	
	pid_t pid = fork();
	
	if(pid == 0) {
		process_order(orders[i]);
		exit(0);
	} else if (pid > 0){
		pids[i] = pid;
		printf("[MANAGER] fork() order #%d → child PID: %d\n", i + 1, pid);
	} else if (pid < 0) {
		perror("fork failed");
		// Cleanup: Thu gom cac tien trinh con da sinh ra truoc do
		for (int j = 0; j < i; j++) {
			waitpid(pids[j], NULL, 0); 
		}
		exit(1);
	}
}

printf("[MANAGER] All %d children spawned. Starting waitpid()...\n\n");
printf("--- [child output order may interleave — this is normal] ---\n\n");

int success_count = 0;

// Cho tien trinh con
for(int i = 0; i < NUM_ORDERS; i++) {
	waitpid(pids[i], &status, 0);
	
	if(i == 0) {
		// In dong separator ngay truoc khi bat dau in ket qua waitpid
		printf("--- [~2 seconds later, all 3 children call exit(0)] ---\n\n");
	}

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

// In bang summary chuan form
printf("\n================= SUMMARY =================\n");
printf("  Total orders    : %d\n", NUM_ORDERS);
printf("  Successful      : %d\n", success_count);
printf("  Failed          : %d\n", NUM_ORDERS - success_count);
printf("  Total revenue   : %'.0f VND\n", total_revenue);
printf("===========================================\n");

return 0;


}