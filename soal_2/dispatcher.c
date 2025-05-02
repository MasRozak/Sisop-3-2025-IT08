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


#define MAX_ORDERS 100
#define MAX_STR 100
#define SHM_KEY 1234
#define FILE_NAME "delivery_order.csv"
#define URL "https://drive.usercontent.google.com/u/0/uc?id=1OJfRuLgsBnIBWtdRXbRsD2sG6NhMKOg9&export=download"


typedef struct {
    char nama[MAX_STR];
    char alamat[MAX_STR];
    char tipe[MAX_STR];   // Express / Reguler
    char status[MAX_STR]; // Pending / Delivered
} Order;

typedef struct {
    Order orders[MAX_ORDERS];
    int count;
} SharedData;

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

void delShm() {
    int shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
    if (shmid != -1) {
        shmctl(shmid, IPC_RMID, NULL);
        printf("Shared memory deleted\n");
    } else {
        perror("shmget failed");
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

        char line[256];
        fgets(line, sizeof(line), fp); // skip header
        data->count = 0;

        while (fgets(line, sizeof(line), fp) && data->count < MAX_ORDERS) {
            sscanf(line, "%[^,],%[^,],%[^\n]",
                data->orders[data->count].nama,
                data->orders[data->count].alamat,
                data->orders[data->count].tipe
                );
            strcpy(data->orders[data->count].status, "Pending");
            data->count++;
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
void printTable(SharedData *shared_data) {
    // HEADER LOGO DALAM TABEL
    printf("+-------------------------------------------------------------------------------------------------+\n");
    printf("|                                                                                                 |\n");
    printf("|         ██████╗ ██╗   ██╗███████╗██╗  ██╗ ██████╗  ██████╗     ███████╗██╗  ██╗██████╗          |\n");
    printf("|         ██╔══██╗██║   ██║██╔════╝██║  ██║██╔════╝ ██╔═══██╗    ██╔════╝╚██╗██╔╝██╔══██╗         |\n");
    printf("|         ██████╔╝██║   ██║███████╗███████║██║  ███╗██║   ██║    █████╗   ╚███╔╝ ██████╔╝         |\n");
    printf("|         ██╔══██╗██║   ██║╚════██║██╔══██║██║   ██║██║   ██║    ██╔══╝   ██╔██╗ ██╔═══╝          |\n");
    printf("|         ██║  ██║╚██████╔╝███████║██║  ██║╚██████╔╝╚██████╔╝    ███████╗██╔╝ ██╗██║              |\n");
    printf("|         ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝ ╚═════╝  ╚═════╝     ╚══════╝╚═╝  ╚═╝╚═╝              |\n");
    printf("|                                                                                                 |\n");
    printf("+-------------------------------------------------------------------------------------------------+\n");

    // HEADER KOLOM
    printf("+----+----------------------+----------------------+----------+-----------------------------------+\n");
    printf("| No | Nama                 | Alamat               | Tipe     | Status                            |\n");
    printf("+----+----------------------+----------------------+----------+-----------------------------------+\n");

    // ISI TABEL
    for (int i = 0; i < shared_data->count; i++) {
        printf("| %2d | %-20s | %-20s | %-8s | %-33s |\n",
               i + 1,
               shared_data->orders[i].nama,
               shared_data->orders[i].alamat,
               shared_data->orders[i].tipe,
               shared_data->orders[i].status);
    }

    // FOOTER
    printf("+----+----------------------+----------------------+----------+-----------------------------------+\n");
    printf("Total: %d order\n", shared_data->count);
}

void wrongForm(){
    printf(" --- Unvalid Command ---\n");
    printf("Correct Format: ./dispatcher -list\n");
    printf("Correct Format: ./dispatcher -deliver <nama>\n");
    printf("Correct Format: ./dispatcher -status <nama>\n");
}

void deliverReg(char *nama, SharedData *shared_data) {
    char *agent_name = getenv("USER");
    int found = 0;
    for (int i = 0; i < shared_data->count; i++) {
        if(strcmp(shared_data->orders[i].nama, nama) ==0 ) {
            if (strcmp(shared_data->orders[i].tipe, "Reguler") != 0) {
                printf("Order %s bukan bertipe Reguler!\n", nama);
                return;
            }

            if (strncmp(shared_data->orders[i].status, "Pending", 7) != 0) {
                printf("Order %s already %s\n", nama, shared_data->orders[i].status);
                return;
            }

            strcpy(shared_data->orders[i].status, "Delivered");
            found = 1;
            sprintf(shared_data->orders[i].status, "Delivered by AGENT %s", agent_name);

            FILE *log = fopen("delivery.log", "a");
            if (log) {
                time_t now = time(NULL);
                struct tm *t = localtime(&now);
                fprintf(log, "[%02d/%02d/%04d %02d:%02d:%02d] [AGENT %s] Reguler package delivered to %s in %s\n",
                    t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                    t->tm_hour, t->tm_min, t->tm_sec,
                    agent_name,
                    shared_data->orders[i].nama,
                    shared_data->orders[i].alamat);
                fclose(log);
                break;
            }
        }
    }
    
    
}
void statusShipment(char *nama, SharedData *shared_data) {
    for (int i = 0; i < shared_data->count; i++) {
        if (strcmp(shared_data->orders[i].nama, nama) == 0) {
            printf("Status for %s: %s\n", nama, shared_data->orders[i].status);
            return;
        }
    }
    printf("Order %s Not Found\n", nama);
}   

int main(int argc, char *argv[]) {
    SharedData *shared_data = init_shared_memory();

    int formtru = -1;
char *nama = NULL;


for (int i = 1; i < argc; i++) {
    if (argc == 2 && strcmp(argv[i], "-list") == 0) {
        formtru = 1;
        printTable(shared_data);
        return 0;
    }
    else if(argc == 2 && strcmp(argv[i], "-rm") == 0) {
        delShm();
        return 0;
    }
    else if (strcmp(argv[i], "-deliver") == 0 || strcmp(argv[i], "-status") == 0) {
        formtru = -1;
        break;
    }
    else if (strcmp(argv[i], "-deliver") == 0 || strcmp(argv[i], "-status") == 0 && i + 1 < argc) {
        formtru = 1;
        break;
    }

    if ((strcmp(argv[i], "-deliver") == 0 || strcmp(argv[i], "-status") == 0) && i + 1 < argc) {
        nama = argv[i + 1];
        formtru = 1;
        break;
    }
}

if (formtru == -1) {
    wrongForm();
    return 0;
} else {
    if (strcmp(argv[1], "-deliver") == 0) {
        deliverReg(nama, shared_data);
    } else if (strcmp(argv[1], "-status") == 0) {
       statusShipment(nama, shared_data);
    }
}


    int shmid = shmget(SHM_KEY, 0, 0666);


    shmdt(shared_data);
    return 0;
}
