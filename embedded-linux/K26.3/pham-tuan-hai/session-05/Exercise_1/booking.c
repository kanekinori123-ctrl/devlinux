#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
	int  agent_id;
	char customer[50];
	int  seats_wanted;
} BookingRequest;

BookingRequest requests[5] = {
	{1, "Nguyen Van An",  2},
	{2, "Tran Thi Bich",  1},
	{3, "Le Van Cuong",   3},
	{4, "Pham Thi Dung",  1},
	{5, "Hoang Van Em",   2}
};

int seats_available = 10;
pthread_mutex_t seat_lock;
int failed_bookings = 0;
int seats_sold = 0;

void*book_ticket(void *arg) {
	BookingRequest* req = (BookingRequest*)arg;

	if (req == NULL) {
	    pthread_exit(NULL);
	}
	sleep(1);
	

	pthread_mutex_lock(&seat_lock);
	printf("[Agent %d | TID %lu...] Booking %d seats for %s...\n", req->agent_id, (unsigned long)pthread_self(), req->seats_wanted, req->customer);
	
	if (seats_available >= req->seats_wanted) {
		seats_available -= req->seats_wanted;
		printf("[Agent %d] CONFIRMED: %d seats for %s. Remaining: %d\n", req->agent_id, req->seats_wanted, req->customer, seats_available);
		seats_sold += req->seats_wanted;
	} else{
		printf("[Agent %d] SOLD OUT:  needs %d seats, only %d left — booking failed.\n", req->agent_id, req->seats_wanted, seats_available);
		failed_bookings++;
  }
	
	if (pthread_mutex_unlock(&seat_lock) != 0) {
		perror("Unlock seat failed");
	}
	pthread_exit(NULL);
}

int main() {
	if(pthread_mutex_init(&seat_lock, NULL)!= 0) {
  	perror("Init lock failed");
		return 1;
  }
	//pthread_create × 5  (pass &requests[i])
	pthread_t threads[5];

	for (int i = 0; i < 5; i++) {
		if (pthread_create(&threads[i], NULL, book_ticket, &requests[i]) != 0) {
			perror("Create thread failed");
			return 1;
		}
	}
	
	usleep(100000); 
	
	printf("==============================================\n");
	printf("   TICKET BOOKING SYSTEM (5 agents, 10 seats)\n");
	printf("==============================================\n");

	// Vòng lặp chờ 5 đại lý xử lý xong
	for (int i = 0; i < 5; i++) {
    // Truyền NULL vì không cần lấy kết quả
    if (pthread_join(threads[i], NULL) != 0) {
			perror("Join thread failed");
		}
	}

	pthread_mutex_destroy(&seat_lock);
	
	printf("\n================ SUMMARY ================\n");
  printf("  Total seats     : 10\n");
  printf("  Seats sold      : %d\n", seats_sold);
  printf("  Seats remaining : %d\n", seats_available);
  printf("  Failed bookings : %d\n", failed_bookings);
  printf("=========================================\n");
	return 0;
}


/*
 * GIẢI THÍCH: TẠI SAO CHECK VÀ DEDUCT PHẢI NẰM TRONG CÙNG 1 BLOCK LOCK?
 * - Hành động "kiểm tra số lượng (Check)" và "trừ đi (Deduct)" phải là 
 *   một khối hành động nguyên tử (atomic).
 * - Nếu chúng ta tách rời chúng ra (ví dụ: Lock -> Check -> Unlock, rồi lại 
 *   Lock -> Deduct -> Unlock), CPU có thể chuyển luồng (context switch) 
 *   ngay tại khe hở giữa 2 lần khóa.
 * - Hậu quả: Cả 2 luồng cùng nhìn thấy còn đúng 1 ghế trống. Sau đó, cả 2 
 *   cùng tiến hành trừ ghế. Dẫn đến tình trạng bán vượt quá số lượng 
 *   (overbooking) và biến số lượng bị âm. Việc gom chung vào 1 block Lock
 *   sẽ ngăn chặn triệt để tình trạng Race Condition này.
 */