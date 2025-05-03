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

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define CYAN "\033[36m"
#define ORANGE "\033[38;5;214m"  

#define MAX_ORDERS 100
#define MAX_STR 100
#define SHM_KEY 1234
#define FILE_NAME "delivery_order.csv"
#define URL "https://drive.usercontent.google.com/u/0/uc?id=1OJfRuLgsBnIBWtdRXbRsD2sG6NhMKOg9&export=download"

typedef struct {
    char nama[MAX_STR];
    char alamat[MAX_STR];
    char tipe[MAX_STR];
    char status[MAX_STR];
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
        fgets(line, sizeof(line), fp);
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
    printf("%s┌──────────────────────────────────────────────────────────────────────────────────────────┐%s\n", CYAN, RESET);  
    printf("%s│                                                                                          │%s\n", CYAN, RESET);
    printf("%s│       %s██████╗ ██╗   ██╗███████╗██╗  ██╗ ██████╗  ██████╗     %s███████╗██╗  ██╗██████╗     %s│%s\n", CYAN, YELLOW, ORANGE, CYAN, RESET);
    printf("%s│       %s██╔══██╗██║   ██║██╔════╝██║  ██║██╔════╝ ██╔═══██╗    %s██╔════╝╚██╗██╔╝██╔══██╗    %s│%s\n", CYAN, YELLOW, ORANGE, CYAN, RESET);
    printf("%s│       %s██████╔╝██║   ██║███████╗███████║██║  ███╗██║   ██║    %s█████╗   ╚███╔╝ ██████╔╝    %s│%s\n", CYAN, YELLOW, ORANGE, CYAN, RESET);
    printf("%s│       %s██╔══██╗██║   ██║╚════██║██╔══██║██║   ██║██║   ██║    %s██╔══╝   ██╔██╗ ██╔═══╝     %s│%s\n", CYAN, YELLOW, ORANGE, CYAN, RESET);
    printf("%s│       %s██║  ██║╚██████╔╝███████║██║  ██║╚██████╔╝╚██████╔╝    %s███████╗██╔╝ ██╗██║         %s│%s\n", CYAN, YELLOW, ORANGE, CYAN, RESET);
    printf("%s│       %s╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝ ╚═════╝  ╚═════╝     %s╚══════╝╚═╝  ╚═╝╚═╝         %s│%s\n", CYAN, YELLOW, ORANGE, CYAN, RESET);
    printf("%s│                                                                                          │%s\n", CYAN, RESET);
    printf("%s└──────────────────────────────────────────────────────────────────────────────────────────┘%s\n", CYAN, RESET);

    printf("%s┌────┬────────────────────┬────────────────────┬──────────┬────────────────────────────────┐%s\n", CYAN, RESET);
    printf("│ No │ %-18s │ %-18s │ %-8s │ %-30s │\n", "Nama", "Alamat", "Tipe", "Status");
    printf("%s├────┼────────────────────┼────────────────────┼──────────┼────────────────────────────────┤%s\n", CYAN, RESET);

    for (int i = 0; i < shared_data->count; i++) {
        const char *status_color = (strcmp(shared_data->orders[i].status, "Pending") == 0) ? RED : GREEN;
        printf("%s│%s %2d %s│%s %-18s %s│%s %-18s %s│%s %-8s %s│%s %s%-30s%s %s│%s\n",
               CYAN, RESET, i + 1, CYAN, RESET,
               shared_data->orders[i].nama, CYAN, RESET,
               shared_data->orders[i].alamat, CYAN, RESET,
               shared_data->orders[i].tipe, CYAN, RESET,
               status_color, shared_data->orders[i].status, RESET, CYAN, RESET);
        }

    printf("%s└────┴────────────────────┴────────────────────┴──────────┴────────────────────────────────┘%s\n", CYAN, RESET);
    printf("Total: %d order %s📝\n", shared_data->count, CYAN);
}
void wrongForm() {
    printf("%s┌──────────────────────────────────────────────────────────┐%s\n", CYAN, RESET);  
    printf("%s│%s                  %-40s%s│%s\n", CYAN, RED, " --- Unvalid Command --- ", CYAN, RESET);  
    printf("%s├──────────────────────────────────────────────────────────┤%s\n", CYAN, RESET);  
    printf("%s│%s%-45s%s             │%s\n", CYAN, GREEN, "Show List Customer: ./dispatcher -list", CYAN, RESET);
    printf("%s│%s%-45s%s    │%s\n", CYAN, GREEN, "Send Package to Customer: ./dispatcher -deliver <nama>", CYAN, RESET);
    printf("%s│%s%-45s%s        │%s\n", CYAN, GREEN, "Check Delivery Status: ./dispatcher -status <nama>", CYAN, RESET);
    printf("%s│%s%-45s%s          │%s\n", CYAN, GREEN, "Delete Data from System Memory: ./dispatcher -rm", CYAN, RESET);
    printf("%s└──────────────────────────────────────────────────────────┘%s\n", CYAN, RESET);  
}
void deliverReg(char *nama, SharedData *shared_data) {
    char *agent_name = getenv("USER");
    int found = 0;
    for (int i = 0; i < shared_data->count; i++) {
        if(strcmp(shared_data->orders[i].nama, nama) ==0 ) {
            if (strcmp(shared_data->orders[i].tipe, "Reguler") != 0) {
                printf("%s❌ Order %s bukan bertipe Reguler! Harap periksa kembali tipe order.%s\n", RED, nama, RESET);
                return;
            }

            if (strncmp(shared_data->orders[i].status, "Pending", 7) != 0) {
                printf("%s⚠️  Order %s has already been marked as %s%s. No further action is required.%s\n", YELLOW, nama, shared_data->orders[i].status, YELLOW, RESET);
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
                printf("%s✈️  Order %s has been %sDELIVERED%s successfully! %s📦%s\n", GREEN, nama, CYAN, GREEN, ORANGE, RESET);

                break;
            }
        }
    }
    
    
}
void statusShipment(char *nama, SharedData *shared_data) {
    for (int i = 0; i < shared_data->count; i++) {
        if (strcmp(shared_data->orders[i].nama, nama) == 0) {
            printf("%s┌──────────────────────────────────────────────────────────────┐%s\n", CYAN, RESET);
            printf("%s│%s                      %-18s%s                   │%s\n", CYAN, GREEN, "📦 Shipment Status 📦", CYAN, RESET);
            printf("%s├──────────────────────────────────────────────────────────────┤%s\n", CYAN, RESET);
            printf("%s│%s %-20s: %-38s %s│%s\n", CYAN, RESET, "Customer Name", shared_data->orders[i].nama, CYAN, RESET);
            printf("%s│%s %-20s: %-38s %s│%s\n", CYAN, RESET, "Address", shared_data->orders[i].alamat, CYAN, RESET);
            const char *type_color = (strcmp(shared_data->orders[i].tipe, "Express") == 0) ? BLUE : ORANGE;
            printf("%s│%s %-20s: %s%-38s%s %s│%s\n", CYAN, RESET, "Order Type", type_color, shared_data->orders[i].tipe, RESET, CYAN, RESET);
            const char *status_color = (strcmp(shared_data->orders[i].status, "Pending") == 0) ? RED : GREEN;
            printf("%s│%s %-20s: %s%-38s%s %s│%s\n", CYAN, RESET, "Status", status_color, shared_data->orders[i].status, RESET, CYAN, RESET);
            printf("%s└──────────────────────────────────────────────────────────────┘%s\n", CYAN, RESET);
            return;
        }
    }
    printf("Order %s Not Found\n", nama);
}   
void delshm(){
    int shmid = shmget(SHM_KEY, 0, 0666);
    shmctl(shmid, IPC_RMID, NULL);
        printf("Shared memory deleted\n");
        exit(0);

}

int main(int argc, char *argv[]) {
    SharedData *shared_data = init_shared_memory();

    int formtru = -1;
char *nama = NULL;


for (int i = 1; i < argc; i++) {
    if (argc == 2) {
        if (strcmp(argv[i], "-list") == 0){
        formtru = 1;
        printTable(shared_data);
        return 0;
        }
        else  if (strcmp(argv[i], "-rm") == 0){ 
            delshm();
        }

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




    shmdt(shared_data);
    return 0;
}
