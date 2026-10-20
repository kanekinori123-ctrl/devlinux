#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {
    int  doc_id;
    char filename[60];
    int  pages;
} Document;

Document queue[5];
int head = 0, tail = 0, count = 0;
int all_sent = 0;

int docs_printed = 0;
int total_pages = 0;

Document mock_data[3][3] = {
    // Producer 1 (id = 1)
    { {1, "report_Q1.pdf", 12}, {4, "slides.pdf", 20}, {7, "summary.pdf", 4} },
    
    // Producer 2 (id = 2)
    { {2, "contract.pdf", 5},   {5, "memo.pdf", 2},    {8, "budget.pdf", 7} },
    
    // Producer 3 (id = 3)
    // (Đề bài hiển thị thiếu 1 file của Prod 3, mình tự bổ sung file 5 trang cho đủ 66 trang)
    { {3, "invoice.pdf", 3},    {6, "proposal.pdf", 8}, {9, "notes.pdf", 5} } 
};

pthread_mutex_t q_lock;
pthread_cond_t not_full;
pthread_cond_t not_empty;

void* produce(void* arg) {
	int my_id = *(int*) arg;
	int index = my_id - 1;
	
	for(int i = 0; i < 3; i++) {
  	Document doc = mock_data[index][i];
		
		pthread_mutex_lock(&q_lock);
		
		while(count == 5 && !all_sent) {
    	pthread_cond_wait(&not_full, &q_lock);
    }
		
		queue[tail] = doc;
		tail = (tail + 1) % 5;
		count++;
		
		printf("[Producer %d] Submitting: %s (%d pages) — queue: %d/5\n", my_id, doc.filename, doc.pages, count);
		
		pthread_cond_signal(&not_empty);
		
		pthread_mutex_unlock(&q_lock);
		
		usleep(100000);
  }
	pthread_exit(NULL);
}

void* print(void *arg) {
	(void)arg;
	
	while(1) {
  	pthread_mutex_lock(&q_lock);
		
		while(count == 0 && !all_sent) {
    	pthread_cond_wait(&not_empty, &q_lock);
    }
		
		if (count == 0 && all_sent) {
    	pthread_mutex_unlock(&q_lock);
      break;
    }
		
		Document doc = queue[head];
		head = (head + 1) % 5;
		count--;
		
		printf("[Printer]    Printing:   %s    (%d pages) — queue: %d/5\n", doc.filename, doc.pages, count);
		
		docs_printed++;
		total_pages += doc.pages;
		
		pthread_cond_signal(&not_full);
		
		pthread_mutex_unlock(&q_lock);
		sleep(1);
  }
	
	printf("[Printer] All documents printed. Exiting.\n");
	
	pthread_exit(NULL);
}

int main() {

	// 1. KHỞI TẠO TÀI NGUYÊN ĐỒNG BỘ (Bắt buộc làm đầu tiên) DONE
		if(pthread_mutex_init(&q_lock)) {
    	perror("Create lock failed");
			return 1;
    }
		
		if(pthread_cond_init(&not_full)){
    	perror("Create cond failed");
			return 1;
    }
		
		if(pthread_cond_init(&not_empty)) {
    	perror("Create cond failed");
			return 1;
    }

    // 2. IN GIAO DIỆN BẮT ĐẦU DONE
		printf("==============================================\n");
		printf("   OFFICE PRINT QUEUE (3 producers, 1 printer)\n");
		printf("   Queue capacity: 5 documents\n");
		printf("==============================================\n");

    // 3. CHUẨN BỊ BIẾN QUẢN LÝ LUỒNG VÀ ĐỊNH DANH
		pthread_t prod_threads[3];
		pthread_t printer_thread;
		
		int prod_ids[3] = {1, 2, 3};

    // 4. PHÂN CÔNG CÔNG VIỆC (Khởi chạy luồng)
		if (pthread_create(&printer_thread, NULL, print, NULL) != 0) {
			perror("Create print failed");
			return 1;
    }
    
		for(int i = 0; i < 3; i++) {
    	if (pthread_create(&prod_threads[i], NULL, produce, &prod_ids[i]) != 0) {
      	perror("Create producer failed");
				return 1;
      }
    }

    // 5. CHỜ NHÂN VIÊN LÀM XONG VIỆC (Giai đoạn đồng bộ 1)
        // Ghi chú trong đầu: Vượt qua được vòng lặp này nghĩa là cả 3 người đã nộp đủ 9 tài liệu và ra về.
				for(int i = 0; i < 3; i++) {
        	pthread_join(prod_threads[i], NULL);
        }
    // 6. CHỐT SỔ VÀ BÁO ĐỘNG
		all_sent = 1;
		pthread_cond_broadcast(&not_empty);
    // Ghi chú trong đầu: Nhỡ đâu máy in đang ngủ nướng vì lúc nãy kho trống, phải bấm chuông gọi nó dậy để thấy cờ chốt sổ mà tắt máy đi về.

    // 7. CHỜ MÁY IN TẮT MÁY (Giai đoạn đồng bộ 2)
    pthread_join(printer_thread, NULL);
    
    // 8. BÁO CÁO THỐNG KÊ
		printf("================ SUMMARY ================\n");
		printf("  Documents submitted : 9\n");
		printf("  Documents printed   : %d\n", docs_printed);
		printf("  Total pages printed : %d\n", total_pages);

    // 9. DỌN DẸP HỘI TRƯỜNG
	pthread_mutex_destroy(&q_lock);
	pthread_cond_destroy(&not_full);
	pthread_cond_destroy(&not_empty);
	
	return 0;
}