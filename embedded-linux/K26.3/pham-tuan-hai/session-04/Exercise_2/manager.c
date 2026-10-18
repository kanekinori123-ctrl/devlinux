#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
extern char **environ;

int main() {  
  while(1){  
  
    printf("=============================================\n");
    printf("STUDENT LOOKUP SYSTEM — MANAGER\n");
    printf("fork + execve | file: students.txt\n");
    printf("=============================================\n");
    
    printf("[MANAGER] PID: %d\n", getpid());
    printf("Enter student ID ('quit' to exit).");
    
    //Nhap ID can tim
    char student_id[20];
    fgets(student_id, sizeof(student_id), stdin);
    student_id[strcspn(student_id, "\n")] = 0;
    
    //Neu nhap "quit"
    if (strcmp(student_id, "quit") == 0) {
      printf("[MANAGER] Exiting. Goodbye!\n");
      return 0;
    }
    
    //Tao fork
    pid_t pid = fork();
    
    //Tien trinh con
    if (pid == 0) {
    
      //Khoi tao args[]
      char *args[] = {
        "./searcher",
        student_id,
        "./students.txt",
        NULL
      };
      // Special case: execve chi tra ve khi no that bai.
			// Neu thanh cong, no se thay the hoan toan tien trinh hien tai.
			// Vi vay, block code nay chi chay khi execve gap loi.
      execve("./searcher", args, environ);
      
      perror("Khong mo duoc file");
      exit(2); // Bat buoc phai co exit de child process khong chay lot xuong logic cua parent
    }
    else if (pid < 0) {
		perror("fork failed");
		exit(1);
	}
    //Tien trinh cha
    else if (pid > 0) {
      //TODO
      int status;
      printf("[MANAGER] fork() → child PID: %d\n", pid);
      printf("[MANAGER] Waiting for child (waitpid)...\n\n");
      
      //Cho tien trinh con
      waitpid(pid, &status, 0);
      
      if(WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        
        //Kiem tra exit_code
        if(exit_code == 0) {
          printf("[MANAGER] Child (PID %d) exited. code=%d → Found\n", pid, exit_code);
        } else if(exit_code == 1) {
          printf("[MANAGER] Child (PID %d) exited. code=%d → Not found\n", pid, exit_code);
        } else if(exit_code == 2) {
          printf("Khong mo duoc file\n");
        }
      }
    }
    printf("----------------------------------\n\n");
    printf("An nut bat ky de tiep tuc\n");
    getchar();
  }
}
