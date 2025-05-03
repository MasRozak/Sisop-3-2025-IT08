#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define PORT 8080
#define BUF_SIZE 1048576

void send_log(int sockfd, const char *action, const char *info) {
    int len = strlen(action);
    write(sockfd, &len, sizeof(int));
    write(sockfd, action, len);
    len = strlen(info);
    write(sockfd, &len, sizeof(int));
    write(sockfd, info, len);
}

void store_file(const char *filename) {
    char path[256];
    snprintf(path, sizeof(path), "client/secrets/%s", filename);
    FILE *fp = fopen(path, "r");
    if (!fp) {
        printf("File '%s' tidak ditemukan.\n", filename);
        return;
    }
    char *content = malloc(BUF_SIZE);
    int len = fread(content, 1, BUF_SIZE, fp);
    fclose(fp); content[len] = '\0';
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = { .sin_family = AF_INET, .sin_port = htons(PORT), .sin_addr.s_addr = inet_addr("127.0.0.1") };
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Gagal connect ke server.\n"); return;
    }
    int command = 1;
    write(sockfd, &command, sizeof(int));
    send_log(sockfd, "DECRYPT", "Text data");
    write(sockfd, &len, sizeof(int));
    write(sockfd, content, len);
    char response[256];
    int n = read(sockfd, response, sizeof(response));
    if(n <= 0){
        printf("Error: Tidak ada respons dari server\n");
        close(sockfd);
        return;
    }
    response[n] = '\0';
    printf("Server menyimpan sebagai: %s\n", response);
    close(sockfd);
    free(content);
}

void fetch_file(const char *filename) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = { .sin_family = AF_INET, .sin_port = htons(PORT), .sin_addr.s_addr = inet_addr("127.0.0.1") };
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Gagal connect ke server.\n"); return;
    }
    int command = 2;
    write(sockfd, &command, sizeof(int));
    send_log(sockfd, "DOWNLOAD", filename);
    int len = strlen(filename);
    write(sockfd, &len, sizeof(int));
    write(sockfd, filename, len);
    int file_len;
    read(sockfd, &file_len, sizeof(int));
    if (file_len == -1) {
        printf("Error: File '%s' tidak ditemukan di server.\n", filename);
        close(sockfd);
        return;
    }
    char *buffer = malloc(file_len);
    read(sockfd, buffer, file_len);
    char path[256];
    snprintf(path, sizeof(path), "client/%s", filename);
    FILE *fp = fopen(path, "wb");
    fwrite(buffer, 1, file_len, fp); fclose(fp);
    printf("File disimpan sebagai %s\n", path);
    free(buffer); close(sockfd);
}

void ensure_directory_structure() {
    struct stat st = {0};

    // Cek apakah 'client' ada dan merupakan direktori
    if (stat("client", &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            printf("Error: 'client' adalah file, bukan folder. Harap hapus file tersebut.\n");
            exit(1);
        }
    } else {
        mkdir("client", 0755);
    }

    // Buat 'client/secrets' jika belum ada
    if (stat("client/secrets", &st) == -1) {
        mkdir("client/secrets", 0755);
    }
}

int main() {
    // Pastikan command curl dan unzip tersedia
    if(system("command -v curl > /dev/null 2>&1") != 0) {
        printf("Error: curl tidak tersedia.\n");
        exit(1);
    }
    if(system("command -v unzip > /dev/null 2>&1") != 0) {
        printf("Error: unzip tidak tersedia.\n");
        exit(1);
    }

    ensure_directory_structure();

    system("curl -L -o secrets.zip \"https://drive.usercontent.google.com/u/0/uc?id=15mnXpYUimVP1F5Df7qd_Ahbjor3o1cVw&export=download\"");
    system("unzip -o secrets.zip -d client");
    system("rm secrets.zip");

    while (1) {
        printf("\n====Image Decoder Client=====\n");
        printf("1. Send input file to server\n");
        printf("2. Download file from server\n");
        printf("3. Exit\n");
        printf(">> ");
        int choice;
        scanf("%d", &choice);
        char filename[256];
        if (choice == 1) {
            printf("Masukkan nama file (input_X.txt): ");
            scanf("%s", filename);
            store_file(filename);
        } else if (choice == 2) {
            printf("Masukkan nama file jpeg (timestamp.jpeg): ");
            scanf("%s", filename);
            fetch_file(filename);
        } else if (choice == 3) {
            int sockfd = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in serv_addr = { .sin_family = AF_INET, .sin_port = htons(PORT), .sin_addr.s_addr = inet_addr("127.0.0.1") };
            if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
                printf("Error: Server gagal dihubungi.\n");
                return 0;
            }
            int command = 3;
            write(sockfd, &command, sizeof(int));
            send_log(sockfd, "EXIT", "Client requested to exit");
            close(sockfd);
            break;
        }
    }
    return 0;
}
