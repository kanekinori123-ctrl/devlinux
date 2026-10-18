#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <student_id> <file_path>\n", argv[0]);
		exit(2); 
	}

	FILE *file = fopen(argv[2], "r");
	if (file == NULL) {
		perror("Loi khong the mo File\n");
		exit(2);
	}	

	char line[256];

	printf("[SEARCHER] PID: %d | PPID: %d\n", getpid(), getppid());
	printf("[SEARCHER] Searching for \"%s\" in students.txt...\n\n", argv[1]);

	while(fgets(line, sizeof(line), file) != NULL) {

		line[strcspn(line, "\n")] = 0;

		char *id = strtok(line, "|");
		if (id == NULL) {
			continue;
		}
		
		char *name = strtok(NULL, "|");
		if (name == NULL) {
			continue;
		}

		char *class_name = strtok(NULL, "|");
		if (class_name == NULL) {
			continue;
		}
		
		char *gpa = strtok(NULL, "|");
		if (gpa == NULL) {
			continue;
		}
		
		char *grade_class;

		//Tim thay ID
		if(strcmp(argv[1], id) == 0) {
			printf("========== SEARCH RESULT ==========\n");
			printf(" ID      : %s\n", id);
			printf(" Name    : %s\n", name);
			printf(" Class   : %s\n", class_name);

			float f_gpa = atof(gpa);
			printf(" GPA     : %.1f\n", f_gpa);

			if(f_gpa >= 8.5) {
				grade_class = "Excellent";
			} else if(f_gpa >= 7) {
				grade_class = "Good";
			} else if(f_gpa >= 5) {
				grade_class = "Average";
			} else {
				grade_class = "Poor";
			}
			printf(" Grade   : %s\n", grade_class);
			printf("====================================\n\n");

			fclose(file);
			exit(0);
		}

	}

	fclose(file);

	printf("[SEARCHER] No student found with ID: %s\n", argv[1]);
	exit(1);
}