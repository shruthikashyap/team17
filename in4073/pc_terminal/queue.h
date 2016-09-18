struct queue_t{
	int16_t capacity;
	int16_t size;
	int16_t first;
	int16_t last;
	struct packet_t *elements;
};

extern struct queue_t *queue;
extern pthread_mutex_t mutex;

struct queue_t* createQueue(int);
void enqueue(struct packet_t);
void* process_dequeue(void *);
void* process_dequeue(void* thread);
void* process_receive_packets(void* thread);
