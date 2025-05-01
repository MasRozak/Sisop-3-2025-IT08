#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_ORDERS 100
#define MAX_STR 100
#define SHM_KEY 1234
#define FILE_NAME "delivery_order.csv"
#define URL "https://drive.usercontent.google.com/u/0/uc?id=1OJfRuLgsBnIBWtdRXbRsD2sG6NhMKOg9&export=download"

typedef struct {
    char nama[MAX_STR];
    char alamat[MAX_STR];
    char tipe[MAX_STR];   // Express / Reguler
    char status[MAX_STR]; // Pending / Delivered by ...
} Order;

typedef struct {
    Order orders[MAX_ORDERS];
    int count;
} SharedData;

sem_t sem; // semaphore global

int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

void download_file() {
    pid_t pid = fork();
    if (pid == 0) {
        char *args[] = {"wget", (char *)URL, "-O", FILE_NAME, NULL};
        execvp("wget", args);
        exit(1);
    } else {
        wait(NULL);
    }
}

SharedData* init_shared_memory() {
    if (!file_exists(FILE_NAME)) {
        printf("File CSV belum ditemukan. Mengunduh...\n");
        download_file();
    }

    int shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
    if (shmid == -1 && errno == ENOENT) {
        printf("Shared memory belum ada. Membuat dan mengisi dari CSV...\n");
        shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
        if (shmid == -1) {
            perror("Gagal membuat shared memory");
            exit(1);
        }

        SharedData *data = (SharedData *) shmat(shmid, NULL, 0);
        if (data == (void *) -1) {
            perror("shmat gagal");
            exit(1);
        }

        FILE *fp = fopen(FILE_NAME, "r");
        if (!fp) {
            perror("Gagal membuka file CSV");
            exit(1);
        }

        char line[256];
        fgets(line, sizeof(line), fp); // skip header
        data->count = 0;

        while (fgets(line, sizeof(line), fp) && data->count < MAX_ORDERS) {
            if (sscanf(line, "%[^,],%[^,],%[^\n]",
                       data->orders[data->count].nama,
                       data->orders[data->count].alamat,
                       data->orders[data->count].tipe) == 3) {
                strcpy(data->orders[data->count].status, "Pending");
                data->count++;
            }
        }

        fclose(fp);
        return data;
    } else if (shmid != -1) {
        SharedData *data = (SharedData *) shmat(shmid, NULL, 0);
        if (data == (void *) -1) {
            perror("shmat gagal");
            exit(1);
        }
        return data;
    } else {
        perror("shmget gagal");
        exit(1);
    }
}

void *agent_thread(void *arg) {
    char *agent_name = (char *) arg;

    int shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
    SharedData *data = (SharedData *) shmat(shmid, NULL, 0);

    for (int i = 0; i < data->count; i++) {
        sem_wait(&sem);

        if (strcmp(data->orders[i].tipe, "Express") == 0 &&
            strncmp(data->orders[i].status, "Pending", 7) == 0) {

            sprintf(data->orders[i].status, "Delivered by %s", agent_name);

            FILE *log = fopen("delivery.log", "a");
            if (log) {
                time_t now = time(NULL);
                struct tm *t = localtime(&now);
                fprintf(log, "[%02d/%02d/%04d %02d:%02d:%02d] [%s] Express package delivered to %s in %s\n",
                    t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                    t->tm_hour, t->tm_min, t->tm_sec,
                    agent_name,
                    data->orders[i].nama,
                    data->orders[i].alamat);
                fclose(log);
            }

            sem_post(&sem);
            sleep(1); // beri delay antar pengiriman
            continue;
        }

        sem_post(&sem);
    }

    shmdt(data);
    return NULL;
}

int main() {
    SharedData *shared_data = init_shared_memory();

    sem_init(&sem, 0, 1); // inisialisasi semaphore

    pthread_t threads[3];
    char *agents[] = {"AGENT A", "AGENT B", "AGENT C"};

    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, agent_thread, agents[i]);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    
    sem_destroy(&sem);
    int shmid = shmget(SHM_KEY, 0, 0666);
    shmdt(shared_data);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
