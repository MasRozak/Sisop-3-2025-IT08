#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/socket.h>

#define PORT 8080
#define BUF_SIZE 1048576

void log_msg(const char *source, const char *action, const char *info) {
    FILE *f = fopen("server/server.log", "a");
    if (f) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(f, "[%s][%s]: [%s] [%s]\n", source, timestamp, action, info);
        fclose(f);
    } else {
        perror("fopen log_msg failed");
    }
}

char* reverse_and_decode(const char* input, size_t* out_len) {
    int len = strlen(input);
    if (len % 2 != 0) return NULL;
    char* reversed = malloc(len + 1);
    if(!reversed){
        perror("malloc reversed failed");
        return NULL;
    }
    for (int i = 0; i < len; i++) reversed[i] = input[len - 1 - i];
    reversed[len] = '\0';
    char* output = malloc(len / 2);
    if(!output){
        perror("malloc output failed");
        free(reversed);
        return NULL;
    }
    for (int i = 0; i < len / 2; i++) sscanf(&reversed[i * 2], "%2hhx", &output[i]);
    *out_len = len / 2;
    free(reversed);
    return output;
}

int main() {
    system("mkdir -p server/database");
    int server_fd, client_fd;
    struct sockaddr_in addr;
    char buffer[BUF_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd<0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

   if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr))< 0){
    perror("bind failed");
    exit(EXIT_FAILURE);
   }

    if(listen(server_fd, 5)<0){
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        if(client_fd < 0){
            perror("accept failed");
            continue;
        }
        int command, action_len, info_len;
        if(read(client_fd, &command, sizeof(int)) < 0){
            perror("read command failed");
            close(client_fd);
            continue;
        }
        if(read(client_fd, &action_len, sizeof(int))<0){
            perror("read ation_len failed");
            close(client_fd);
            continue;
        }
        read(client_fd, buffer, action_len); buffer[action_len] = '\0';
        char log_action[64]; strcpy(log_action, buffer);
        read(client_fd, &info_len, sizeof(int));
        read(client_fd, buffer, info_len); buffer[info_len] = '\0';
        char log_info[256]; strcpy(log_info, buffer);
        log_msg("Client", log_action, log_info);

        if (command == 1) {
            int text_len;
            read(client_fd, &text_len, sizeof(int));
            read(client_fd, buffer, text_len); buffer[text_len] = '\0';
            size_t out_len;
            char* decoded = reverse_and_decode(buffer, &out_len);
            if (!decoded) {
                write(client_fd, "ERROR", 5); close(client_fd); continue;
            }
            time_t t = time(NULL);
            char filename[64]; snprintf(filename, sizeof(filename), "%ld.jpeg", t);
            char path[128]; snprintf(path, sizeof(path), "server/database/%s", filename);
            FILE *fp = fopen(path, "wb");
            fwrite(decoded, 1, out_len, fp); fclose(fp);
            free(decoded);
            write(client_fd, filename, strlen(filename));
            log_msg("Server", "SAVE", filename);
        } else if (command == 2) {
            int name_len;
            read(client_fd, &name_len, sizeof(int));
            read(client_fd, buffer, name_len); buffer[name_len] = '\0';
            char buffer[100];
            char filepath[128]; 
            snprintf(filepath, sizeof(filepath), "server/database/%s", buffer);
            FILE *fp = fopen(filepath, "rb");
            if (!fp) { int err = -1; write(client_fd, &err, sizeof(int)); close(client_fd); continue; }
            int file_len = fread(buffer, 1, BUF_SIZE, fp); fclose(fp);
            write(client_fd, &file_len, sizeof(int));
            write(client_fd, buffer, file_len);
            log_msg("Server", "UPLOAD", buffer);
        } else if (command == 3) {
            log_msg("Server", "EXIT", "Client requested to exit");
        }
        close(client_fd);
    }
    return 0;
}
