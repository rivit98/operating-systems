// kolejki komunikatow SYSTEM V
key_t key = ftok(const char *pathname, int proj_id);
int msgid = msgget(key_t key, IPC_CREAT);

struct msgbuf {
	long mtype;       /* message type, must be > 0 */
	char mtext[1];    /* message data */
};
//dodaje do kolejki wiadomosc, msgsz powinno byc dlugoscia wiadomosci bez jej typu
msgsnd(int msqid, const void *msgp, size_t msgsz, 0);
// pobiera z kolejki wiadomosc podanego typu (msgtyp) lub pierwsza (msgtyp=0)
// msgflg=IPC_NOWAIT znaczy ze nie blokuje i jesli errno=ENOMSG to nie ma obecnie zadnej wiadomosci
ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
msgctl(msgid, IPC_RMID, NULL); //usuwa kolejke


// kolejki komunikatow POSIX
struct mq_attr attr;
attr.mq_flags = 0;
attr.mq_maxmsg = 10;
attr.mq_msgsize = MAX_MESSAGE_LEN;
attr.mq_curmsgs = 0;
int msgid = mq_open("/server", O_RDWR | O_CREAT, 0666, &attr /*NULL*/);

mq_close(msgid);
mq_unlink(fname); //usuwa kolejke

struct sigevent e;
e.sigev_notify_function = process_message;
e.sigev_notify = SIGEV_THREAD;
e.sigev_value.sival_ptr = NULL;
e.sigev_notify_attributes = NULL;
mq_notify(client_queue_id, &e); //rejestruje event, ktory zostanie wywolany po otrzymaniu wiadomosci

mq_receive(msgid, &message[0], MAX_MESSAGE_LEN, NULL);
mq_send(msgid, &buffer[0], length, 0);


// semafory SYSTEM V
semaphore_id = semget(key_t key, 4/*ile semaforow*/, IPC_CREAT | 0666);
semctl(semaphore_id, 1, SETVAL, (union semun) {.val = 0}); //ustawia parametry semafora
semctl(semaphore_id, 0, IPC_RMID); //usuwamy semafory
//zajmujemy semafor o numerze 2, drugi parametr to struktury typu:
struct sembuf increase_worker2 = {1, 1, 0}; //pierwsza wartosc id semafora, druga wartosc o jaka zwiekszyc/zmniejszyc
//jesli zajecie semafora powoduje ze jego wartosc jest mniejsza od 0 to proces musi poczekac
semop(semaphore_id, (struct sembuf[2]){decrease_available_products, mem_lock}, 2);


// semafory posix
sem_t = sem_open("/tasks", O_CREAT, 0666, MAX_TASKS);
sem_close(sem_t);
sem_unlink("/tasks");
sem_wait(sem_t); //probuje zajac semafor
sem_post(sem_t); //oddaje semafor


// shared memory SYSTEM V
shmget(sem_key, sizeof(orders_mem), IPC_CREAT | PERMISSION); //wyzerowac po stworzeniu
orders_mem *mem = shmat(shared_memory_id, NULL, 0); //podpinamy pamiec
shmdt((const void *) mem); //odpinamy
shmctl(shared_memory_id, IPC_RMID, 0); //usuwamy


// shared memory posix
int shared_memory_id = shm_open("/shm_mem", O_CREAT | O_RDWR, 0666);
ftruncate(shared_memory_id, sizeof(orders_mem)); //ustawia rozmiar pamieci
orders_mem *mem = mmap(NULL, sizeof(orders_mem), PROT_WRITE | PROT_READ, MAP_SHARED, shared_memory_id, 0); //attach
munmap(mem, sizeof(orders_mem)); //detach
shm_unlink("/shm_mem");


//watki
pthread_create(pthread_t *thread, NULL, (void *(*)(void *)) routine, void *arg);
long ret;
pthread_join(pthread_t *thread, (void *) &ret);

pthread_mutex_t queue_mutex        = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t somebody_in_queue   = PTHREAD_COND_INITIALIZER;

pthread_mutex_lock(&queue_mutex);
while(cond not met){
	pthread_cond_wait(&somebody_in_queue, &queue_mutex);
}
pthread_mutex_unlock(&queue_mutex);


pthread_cond_signal(&somebody_in_queue);
pthread_cond_broadcast(&somebody_in_queue);


// sockety
