# Sisop-3-2025-IT08

Anggota IT08:
| Nama | NRP |
| ---------------------- | ---------- |
| Arya Bisma Putra Refman | 5027241036 |
| Moch. Rizki Nasrullah | 5027241038 |
| Nadia Fauziazahra Kusumastuti | 5027241094 |
----

## Soal 1
[Author: Amoes / wintertia]

The Legend of Rootkids
Sekarang tahun 2045, seluruh dunia mengalami kekacauan dalam segala infrastruktur siber. Sebagai seorang mahasiswa Departemen Teknologi Informasi ITS, anda memiliki ide untuk kembali ke masa lalu (tahun 2025) untuk memanggil hacker asal Semarang bernama “rootkids” yang mampu melawan segala hacker lain. Tetapi anda tidak tahu bagaimana cara mencapainya.
Hanya sedikit yang diketahui tentang hacker bernama “rootkids”. Beberapa informasi yang anda temukan dari deep web hanyalah berupa beberapa file text yang berisi tulisan aneh, beserta beberapa petunjuk untuk mengubah text tersebut menjadi sebuah file jpeg.
Karena anda adalah seorang professional programmer, anda mengikuti petunjuk yang anda dapatkan dari deep web untuk membuat sistem RPC server-client untuk mengubah text file sehingga bisa dilihat dalam bentuk file jpeg. Situs deep web yang anda baca terlihat sebagai berikut.

### a. Text file rahasia terdapat pada [LINK BERIKUT](https://drive.google.com/file/d/15mnXpYUimVP1F5Df7qd_Ahbjor3o1cVw/view), diperbolehkan untuk download/unzip secara manual. 

Sebelum mendownload dan mengunzip file, buat fungsi untuk memastikan bahwa direktori client/secrets tempat menaruh hasil unzip telah tersedia. Dalam hal ini menggunakan fungsi `ensure_directory_structure`

```bash
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
```

Untuk mendownload file dan mengunzip file, dilakukan secara otomatis saat program image_client.c dijalankan

```bash
    ensure_directory_structure();

    system("curl -L -o secrets.zip \"https://drive.usercontent.google.com/u/0/uc?id=15mnXpYUimVP1F5Df7qd_Ahbjor3o1cVw&export=download\"");
    system("unzip -o secrets.zip -d client");
    system("rm secrets.zip");
```

### b. Pada image_server.c, program yang dibuat harus berjalan secara daemon di background dan terhubung dengan image_client.c melalui socket RPC.

Berikut adalah code untuk membaca pesan dari image_client.c dan juga memasukkannya ke dalam activity log

```bash
        read(client_fd, buffer, action_len); buffer[action_len] = '\0';
        char log_action[64]; strcpy(log_action, buffer);
        read(client_fd, &info_len, sizeof(int));
        read(client_fd, buffer, info_len); buffer[info_len] = '\0';
        char log_info[256]; strcpy(log_info, buffer);
        log_msg("Client", log_action, log_info);
```


### c. Program image_client.c harus bisa terhubung dengan image_server.c dan bisa mengirimkan perintah untuk:
Decrypt text file yang dimasukkan dengan cara Reverse Text lalu Decode from Hex, untuk disimpan dalam folder database server dengan nama file berupa timestamp dalam bentuk angka, misalnya: database/1744401282.jpeg
Request download dari database server sesuai filename yang dimasukkan, misalnya: 1744401282.jpeg
**Note: tidak diperbolehkan copy/pindah file, gunakan RPC untuk mengirim data.**

Untuk memulai decrypt file terlebih dahulu membuat fungsi untuk reverse dan decode dari hex. Untuk itu kami menggunakan fungsi `reverse_and_decode()`

```bash
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
```

Fungsi untuk menyimpan file melakukan fetch sehingga yang tersimpan di dalam `client/secrets` adalah nama file yang sudah terdecrypt.

```bash
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


```

### d. Program image_client.c harus disajikan dalam bentuk menu kreatif yang memperbolehkan pengguna untuk memasukkan perintah berkali-kali.

Kondisi pengulangan program secara default adalah benar, yang mana mengizinkan program terus berjalan sebelum user memanggil program untuk berhenti

```bash
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

```

### e. Program dianggap berhasil bila pengguna dapat mengirimkan text file dan menerima sebuah file jpeg yang dapat dilihat isinya.

Server akan menyimpan file `txt` dari client di dalam database dengan format nama `timestamp.jpeg`. Sementara Untuk perintah no. 2 adalah untuk mendownload serta menampilkan file `jpeg` ke client.

```bash
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

```

### f. Program `image_server.c` diharuskan untuk tidak keluar/terminate saat terjadi error dan client akan menerima error message sebagai response, yang meliputi minimal:

| Sumber      | Error                                      |
|-------------|--------------------------------------------|
| Client      | Gagal connect ke server                    |
| Client      | Salah nama text file input                 |
| Server      | Gagal menemukan file untuk dikirim ke client |

Berikut beberapa error handling yang telah dibuat:

```bash
 //Error handling dalam client
    FILE *fp = fopen(path, "r");
    if (!fp) {
        printf("File '%s' tidak ditemukan.\n", filename);
        return;
    }
===============================================
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = { .sin_family = AF_INET, .sin_port = htons(PORT), .sin_addr.s_addr = inet_addr("127.0.0.1") };
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Gagal connect ke server.\n"); return;
    }
```

```bash
//Error handling dalam server
 int sockfd = socket(AF_INET, SOCK_STREAM, 0);
 int file_len;
    read(sockfd, &file_len, sizeof(int));
    if (file_len == -1) {
        printf("Error: File '%s' tidak ditemukan di server.\n", filename);
        close(sockfd);
        return;
    }

```


### g. Server menyimpan log semua percakapan antara image_server.c dan image_client.c di dalam file server.log dengan format: [Source][YYYY-MM-DD hh:mm:ss]: [ACTION] [Info]

Fungsi untuk membuat log sesuai template:

```bash
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
```


## Soal 2
[Author: Nayla / naylaarr]

Tahun 2025, di tengah era perdagangan serba cepat, berdirilah sebuah perusahaan ekspedisi baru bernama RushGo. RushGo ingin memberikan layanan ekspedisi terbaik dengan 2 pilihan, Express (super cepat) dan Reguler (standar). Namun, pesanan yang masuk sangat banyak! Mereka butuh sebuah sistem otomatisasi pengiriman, agar agen-agen mereka tidak kewalahan menangani pesanan yang terus berdatangan. Kamu ditantang untuk membangun Delivery Management System untuk RushGo.

### A. Mengunduh File Order dan Menyimpannya ke Shared Memory
Untuk memulai, Anda perlu mengelola semua orderan yang masuk dengan menggunakan shared memory.
Unduh file `delivery_order.csv`
Setelah file CSV diunduh, program Anda harus membaca seluruh data dari CSV dan menyimpannya ke dalam shared memory.

```bash
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
```
Fungsi `download_file` menggunakan `fork` untuk membuat proses anak yang menjalankan perintah `wget` untuk mengunduh file CSV dari URL yang diberikan. Setelah proses anak selesai, proses induk menunggu hingga unduhan selesai dengan `wait(NULL)`.

```bash

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
```
Fungsi `init_shared_memory` memeriksa apakah file CSV sudah ada. Jika tidak, ia mengunduhnya menggunakan fungsi `download_file`. Kemudian, ia mencoba untuk mendapatkan ID shared memory dengan `shmget`. Jika shared memory belum ada, ia membuatnya dan mengisi data dari file CSV ke dalam shared memory. Jika shared memory sudah ada, ia hanya mengaitkannya dengan `shmat`.

### B. Pengiriman Bertipe Express
RushGo memiliki tiga agen pengiriman utama: AGENT A, AGENT B, dan AGENT C.
Setiap agen dijalankan sebagai thread terpisah.
Agen-agen ini akan secara otomatis:
Mencari order bertipe Express yang belum dikirim.
Mengambil dan mengirimkannya tanpa intervensi user.
Setelah sukses mengantar, program harus mencatat log di delivery.log dengan format:

`[dd/mm/yyyy hh:mm:ss] [AGENT A/B/C] Express package delivered to [Nama] in [Alamat]`

```bash

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

```
Fungsi `agent_thread` adalah fungsi yang dijalankan oleh setiap thread agen. Setiap agen akan mencari order bertipe Express yang belum dikirim, mengubah statusnya menjadi "Delivered by [Nama Agen]", dan mencatat log pengiriman ke dalam file `delivery.log`. Setelah itu, agen akan tidur selama 1 detik sebelum melanjutkan pencarian order berikutnya.

![Image](https://github.com/user-attachments/assets/91e76fa6-e21b-4930-8d62-d9e435ca687b)

### C. Pengiriman Bertipe Reguler
Berbeda dengan Express, untuk order bertipe Reguler, pengiriman dilakukan secara manual oleh user.
User dapat mengirim permintaan untuk mengantar order Reguler dengan memberikan perintah deliver dari dispatcher. 
Penggunaan:
./dispatcher -deliver [Nama]
Pengiriman dilakukan oleh agent baru yang namanya adalah nama user.
Setelah sukses mengantar, program harus mencatat log di delivery.log dengan format:
[dd/mm/yyyy hh:mm:ss] [AGENT <user>] Reguler package delivered to [Nama] in [Alamat] 

```bash
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
```
Fungsi `deliverReg` digunakan untuk mengirimkan order bertipe Reguler secara manual oleh user. Fungsi ini mencari order berdasarkan nama yang diberikan, memeriksa apakah order tersebut bertipe Reguler dan belum dikirim, lalu mengubah statusnya menjadi "Delivered by AGENT [Nama User]". Setelah itu, fungsi mencatat log pengiriman ke dalam file `delivery.log`.

Jika order tidak ditemukan atau sudah dikirim, fungsi akan memberikan pesan kesalahan yang sesuai.

![Image](https://github.com/user-attachments/assets/3556e68b-0e5c-4f28-aa72-ac6d574b0dde)

### D. Mengecek Status Pesanan
Dispatcher juga harus bisa mengecek status setiap pesanan.
Penggunaan:

`./dispatcher -status [Nama]`
```bash
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
```
Fungsi `statusShipment` digunakan untuk mengecek status pesanan berdasarkan nama yang diberikan. Fungsi ini mencari order dalam shared memory dan menampilkan informasi pesanan seperti nama pelanggan, alamat, tipe order, dan status pengiriman. Jika order tidak ditemukan, fungsi akan memberikan pesan bahwa order tidak ditemukan.

![Image](https://github.com/user-attachments/assets/d7e40b81-8a44-4189-925a-e999b9575b21)

### E. Melihat Daftar Semua Pesanan
Untuk memudahkan monitoring, program dispatcher bisa menjalankan perintah list untuk melihat semua order disertai nama dan statusnya.
Penggunaan:

`./dispatcher -list`


```bash
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
```
Fungsi `printTable` digunakan untuk menampilkan semua order dalam bentuk tabel. Fungsi ini mencetak header tabel, lalu mengiterasi semua order dalam shared memory dan mencetak informasi masing-masing order. Tipe dan status order diberi warna yang sesuai untuk memudahkan pembacaan.

![Image](https://github.com/user-attachments/assets/a264016c-d9b4-4a56-a46a-7960221a0449)

### Menghentikan Program 
Menggunakan Command ./dispatcher -rm akan menghapus shared memory dan semaphore yang telah dibuat sebelumnya
```bash
void delshm(){
    int shmid = shmget(SHM_KEY, 0, 0666);
    shmctl(shmid, IPC_RMID, NULL);
        printf("Shared memory deleted\n");
        exit(0);

}
```
Fungsi `delshm` digunakan untuk menghapus shared memory yang telah dibuat sebelumnya. Fungsi ini mendapatkan ID shared memory dengan `shmget`, lalu menghapusnya dengan `shmctl`. Setelah itu, program akan keluar.

![Image](https://github.com/user-attachments/assets/4179e894-f792-4993-a1ef-c358a0b89792)

## Soal 3
[Author: Fico / purofuro]

The Lost Dungeon 
Suatu pagi, anda menemukan jalan setapak yang ditumbuhi lumut dan hampir tertutup semak. Rasa penasaran membawamu mengikuti jalur itu, hingga akhirnya anda melihatnya: sebuah kastil tua, tertutup akar dan hampir runtuh, tersembunyi di tengah hutan. Gerbangnya terbuka seolah memanggilmu masuk.

Di dalam, anda menemukan pintu batu besar dengan simbol-simbol aneh yang terasa… hidup. Setelah mendorongnya dengan susah payah, anda pun menuruni tangga batu spiral yang dalam dan gelap. Di ujungnya, anda menemukan sebuah dunia baru: dungeon bawah tanah yang sudah tertinggal sangat lama.

Anda tidak tahu bagaimana anda dapat berada di situasi ini, tetapi didorong oleh rasa ingin tahu dan semangat, apa pun yang menunggumu di bawah sana, anda akan melawan. 

a. Entering the dungeon -> 
`dungeon.c` akan bekerja sebagai server yang dimana client (`player.c`) dapat terhubung melalui `RPC`. `dungeon.c` akan memproses segala perintah yang dikirim oleh `player.c`. Lebih dari 1 client dapat mengakses server.

```bash
// dungeon.c (Server setup in main)
int main(int argc, char *argv[]) {
    srand(time(NULL));
    init_shop();

    if (argc > 1 && strcmp(argv[1], "local") == 0) {
        Player local_player = {"local", 500, "Fists", 5, 0, {0}, 0};
        main_menu(&local_player);
        return 0;
    }

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    if (listen(server_sock, 10) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf(ANSI_CYAN "🌌 Server running on port 8080...\n" ANSI_RESET);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_t thread;
        int *client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = client_sock;
        if (pthread_create(&thread, NULL, handle_client, client_sock_ptr) != 0) {
            perror("Thread creation failed");
            close(client_sock);
            free(client_sock_ptr);
        }
        pthread_detach(thread);
    }

    close(server_sock);
    return 0;
}

// handle_client function (partial)
void* handle_client(void* arg) {
    int client_sock = *(int*)arg;
    free(arg);
    char buffer[MAX_MESSAGE];
    char response[MAX_MESSAGE * 2];
    int enemy_hp = 0, enemy_max_hp = 0;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int n = read(client_sock, buffer, sizeof(buffer) - 1);
        if (n <= 0) break;

        buffer[n] = '\0';
        char command[20], param[50];
        sscanf(buffer, "%s %s", command, param);

        Player *p = get_player_session(param);
        if (!p) {
            snprintf(response, sizeof(response), "Session not found.\n");
            write(client_sock, response, strlen(response));
            continue;
        }
        // Command processing (GET_STATS, GET_SHOP, etc.)
        // ...
    }

    close(client_sock);
    return NULL;
}
```
Server di `dungeon.c` menggunakan soket TCP untuk menangani beberapa koneksi client secara bersamaan. Fungsi `main`:
1. Membuat soket dengan `socket(AF_INET, SOCK_STREAM, 0)` untuk komunikasi TCP.
2. Mengikatnya ke port 8080 dengan `bind` dan mengatur untuk mendengarkan hingga 10 client dengan `listen`.
3. Menerima koneksi client dalam loop tak terbatas menggunakan `accept`. Setiap koneksi client membuat thread baru melalui `pthread_create` yang memanggil `handle_client` untuk memproses perintah client.
4. Jika program dijalankan dengan argumen "`local`", server dilewati dan menjalankan game lokal dengan `main_menu`.
Struktur ini memungkinkan beberapa client (`player.c`) terhubung ke server, mengirim perintah, dan menerima respons, mendukung lingkungan game dungeon multipemain.

b. Sightseeing -> 
Anda melihat disekitar dungeon dan menemukan beberapa hal yang menarik seperti toko senjata dan pintu dengan aura yang cukup seram. Ketika `player.c` dijalankan, ia akan terhubung ke `dungeon.c` dan menampilkan sebuah main menu seperti yang dicontohkan di bawah ini (tidak harus mirip, dikreasikan sesuai kreatifitas masing-masing praktikan).

```bash
// dungeon.c
void main_menu(Player *p) {
    char input[10];
    int choice;
    while (1) {
        printf(ANSI_GREEN "┌────────────────────────────────────┐\n" ANSI_RESET);
        printf(ANSI_GREEN "│" ANSI_YELLOW "       🏰 LOST DUNGEON 🏰           " ANSI_GREEN "│\n" ANSI_RESET);
        printf(ANSI_GREEN "├────────────────────────────────────┤\n" ANSI_RESET);
        printf(ANSI_GREEN "│ " ANSI_CYAN "[1] Hero's Journal (Stats)   📜" ANSI_RESET " " ANSI_GREEN "│\n" ANSI_RESET);
        printf(ANSI_GREEN "│ " ANSI_CYAN "[2] Mysterious Armory (Shop) 🛒" ANSI_RESET " " ANSI_GREEN "│\n" ANSI_RESET);
        printf(ANSI_GREEN "│ " ANSI_CYAN "[3] Hero's Satchel (Inventory)🎒" ANSI_RESET " " ANSI_GREEN "│\n" ANSI_RESET);
        printf(ANSI_GREEN "│ " ANSI_CYAN "[4] The Dark Gate (Battle)   ⚔" ANSI_RESET " " ANSI_GREEN "│\n" ANSI_RESET);
        printf(ANSI_GREEN "│ " ANSI_CYAN "[5] Flee the Dungeon (Exit)  🏃" ANSI_RESET " " ANSI_GREEN "│\n" ANSI_RESET);
        printf(ANSI_GREEN "└────────────────────────────────────┘\n" ANSI_RESET);
        printf(ANSI_YELLOW "➤ Choose your path: " ANSI_RESET);
        fgets(input, sizeof(input), stdin);
        sscanf(input, "%d", &choice);
        switch (choice) {
            case 1: show_player_stats(p); break;
            case 2: display_shop(p); break;
            case 3: view_inventory(p); break;
            case 4: battle_mode(p); break;
            case 5:
                printf(ANSI_YELLOW "🏃 You escape the dungeon's grasp... for now.\n" ANSI_RESET);
                return;
            default:
                printf(ANSI_RED "⚠ The dungeon whispers: 'Choose wisely!'\n" ANSI_RESET);
                break;
        }
    }
}
```
Fungsi `main_menu` di `dungeon.c` (dan serupa di `player.c` untuk mode lokal) adalah antarmuka utama game. Menu ditampilkan dengan kode warna ANSI, menawarkan lima opsi:
- Hero's Journal (Stats): Memanggil `show_player_stats` untuk menampilkan statistik pemain.
- Mysterious Armory (Shop): Memanggil `display_shop` untuk berinteraksi dengan toko senjata.
- Hero's Satchel (Inventory): Memanggil `view_inventory` untuk melihat dan mengequip senjata.
- The Dark Gate (Battle): Memanggil `battle_mode` untuk memulai pertempuran.
- Flee the Dungeon (Exit): Keluar dari menu dan mengakhiri sesi.
Input pengguna dibaca dengan `fgets` dan diparsing dengan `sscanf`. Pernyataan `switch` mengarahkan ke fungsi yang sesuai. Input tidak valid memicu pesan kesalahan, memastikan penanganan kesalahan yang kuat. Menu ini dapat diakses dalam mode lokal atau client-server, di mana `player.c` mengirim perintah ke `dungeon.c`.

c. Status Check ->
Melihat bahwa terdapat sebuah toko senjata, anda mengecek status diri anda dengan harapan anda masih memiliki sisa uang untuk membeli senjata. Jika opsi Show Player Stats dipilih, maka program akan menunjukan Uang yang dimiliki (Jumlah dibebaskan), senjata yang sedang digunakan, Base Damage, dan jumlah musuh yang telah dimusnahkan. 

```bash
// dungeon.c
void show_player_stats(Player *p) {
    printf(ANSI_CYAN "┌────────────────────────────────────┐\n" ANSI_RESET);
    printf(ANSI_CYAN "│" ANSI_YELLOW "         ⚔ HERO'S JOURNAL ⚔         " ANSI_CYAN "│\n" ANSI_RESET);
    printf(ANSI_CYAN "├────────────────────────────────────┤\n" ANSI_RESET);
    printf(ANSI_CYAN "│ " ANSI_YELLOW "Gold: %-5d 💰" ANSI_RESET "                   " ANSI_CYAN "│\n", p->gold);
    printf(ANSI_CYAN "│ " ANSI_GREEN "Weapon: %-20s" ANSI_RESET " " ANSI_CYAN "│\n", p->equipped_weapon);
    printf(ANSI_CYAN "│ " ANSI_RED "Damage: %-3d ⚡" ANSI_RESET "                 " ANSI_CYAN "│\n", p->base_damage);
    printf(ANSI_CYAN "│ " ANSI_BLUE "Kills: %-4d ☠" ANSI_RESET "                  " ANSI_CYAN "│\n", p->kills);
    for (int i = 0; i < WEAPON_COUNT; i++) {
        if (strcmp(p->equipped_weapon, shop_weapons[i].name) == 0 && strlen(shop_weapons[i].passive) > 0) {
            printf(ANSI_CYAN "│ " ANSI_PURPLE "Passive: %-19s" ANSI_RESET " " ANSI_CYAN "│\n", shop_weapons[i].passive);
        }
    }
    printf(ANSI_CYAN "└────────────────────────────────────┘\n\n" ANSI_RESET);
}
```
Fungsi `show_player_stats` menampilkan status pemain dalam tabel terformat, menunjukkan:
- Gold: Uang pemain (dimulai dari 500).
- Weapon: Senjata yang sedang digunakan (default "Fists").
- Damage: Base damage senjata.
- Kills: Jumlah musuh yang dikalahkan.
- Passive: Jika senjata memiliki kemampuan pasif (misalnya, "30% Crit Chance" untuk Dragon Claws), ditampilkan.
Fungsi ini memeriksa apakah senjata memiliki pasif dengan mengiterasi `shop_weapons`. Dalam mode client-server, perintah `GET_STATS` mengirim informasi serupa ke client. Ini memastikan pemain dapat memeriksa status secara konsisten baik lokal maupun jarak jauh.

d. Weapon Shop -> 
Ternyata anda memiliki sisa uang dan langsung pergi ke toko senjata tersebut untuk membeli senjata. Terdapat 5 pilihan senjata di toko tersebut dan beberapa dari mereka memiliki passive yang unik. Disaat opsi Shop dipilih, program akan menunjukan senjata apa saja yang dapat dibeli beserta harga, damage, dan juga passive (jika ada). List senjata yang ada dan dapat dibeli beserta logic/command untuk membeli senjata tersebut diletakan di `code shop.c/shop.h` yang nanti akan dipakai oleh `dungeon.c`.

```bash
// shop.c
void init_shop() {
    shop_weapons[0] = (Weapon){1, "Terra Blade", 50, 10, ""};
    shop_weapons[1] = (Weapon){2, "Flint & Steel", 150, 25, ""};
    shop_weapons[2] = (Weapon){3, "Kitchen Knife", 200, 35, ""};
    shop_weapons[3] = (Weapon){4, "Staff of Light", 120, 20, "10% Insta-Kill Chance"};
    shop_weapons[4] = (Weapon){5, "Dragon Claws", 300, 50, "30% Crit Chance"};
}

void display_shop(Player *p) {
    char input[10];
    int choice;
    printf(ANSI_PURPLE "┌─────────────────────────────────────────────┐\n" ANSI_RESET);
    printf(ANSI_PURPLE "│" ANSI_YELLOW "         🛒 MYSTERIOUS ARMORY 🛒             " ANSI_PURPLE "│\n" ANSI_RESET);
    printf(ANSI_PURPLE "├────┬──────────────────┬──────┬──────┬────────┤\n" ANSI_RESET);
    printf(ANSI_PURPLE "│" ANSI_CYAN " ID " ANSI_PURPLE "│" ANSI_CYAN " Name             " ANSI_PURPLE "│" ANSI_CYAN " Gold " ANSI_PURPLE "│" ANSI_CYAN " Dmg  " ANSI_PURPLE "│" ANSI_CYAN " Passive" ANSI_PURPLE "│\n" ANSI_RESET);
    printf(ANSI_PURPLE "├────┼──────────────────┼──────┼──────┼────────┤\n" ANSI_RESET);
    for (int i = 0; i < WEAPON_COUNT; i++) {
        Weapon *w = &shop_weapons[i];
        printf(ANSI_PURPLE "│" ANSI_CYAN " %-2d " ANSI_PURPLE "│" ANSI_GREEN " %-16s " ANSI_PURPLE "│" ANSI_YELLOW " %-4d " ANSI_PURPLE "│" ANSI_RED " %-4d " ANSI_PURPLE "│" ANSI_BLUE " %-6s " ANSI_PURPLE "│\n", 
               w->id, w->name, w->price, w->damage, w->passive);
    }
    printf(ANSI_PURPLE "└────┴──────────────────┴──────┴──────┴────────┘\n" ANSI_RESET);
    printf(ANSI_YELLOW "💬 The merchant grins: 'Choose wisely, adventurer!'\n" ANSI_RESET);
    printf(ANSI_CYAN "➤ Enter weapon number to buy (0 to leave): " ANSI_RESET);
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%d", &choice);
    if (choice == 0) {
        printf(ANSI_YELLOW "💬 The merchant nods: 'Come back anytime!'\n" ANSI_RESET);
        return;
    }
    int result = buy_weapon(p, choice);
    if (result == 0) {
        printf(ANSI_GREEN "✅ Successfully bought %s!\n" ANSI_RESET, p->equipped_weapon);
    } else if (result == -1) {
        printf(ANSI_RED "⚠ Invalid weapon ID.\n" ANSI_RESET);
    } else if (result == -2) {
        printf(ANSI_RED "⚠ The merchant laughs: 'Not enough gold, hero!'\n" ANSI_RESET);
    } else if (result == -3) {
        printf(ANSI_RED "⚠ You already own this weapon!\n" ANSI_RESET);
    } else {
        printf(ANSI_RED "⚠ Inventory full!\n" ANSI_RESET);
    }
}
```
Toko senjata diimplementasikan di `shop.c`. Fungsi `init_shop` menginisialisasi lima senjata:
- Terra Blade: 50 gold, 10 damage, tanpa passive.
- Flint & Steel: 150 gold, 25 damage, tanpa passive.
- Kitchen Knife: 200 gold, 35 damage, tanpa passive.
- Staff of Light: 120 gold, 20 damage, 10% peluang instan kill.
- Dragon Claws: 300 gold, 50 damage, 30% peluang critical.
Fungsi `display_shop` menampilkan tabel senjata dengan ID, nama, harga, damage, dan passive. Pemain memasukkan ID senjata untuk membeli atau 0 untuk keluar. Fungsi `buy_weapon` menangani logika pembelian, memeriksa ID valid, kecukupan gold, kepemilikan, dan kapasitas inventaris. Penanganan kesalahan memberikan umpan balik untuk ID tidak valid, gold kurang, atau inventaris penuh.

e. Handy Inventory -> 
Setelah membeli senjata di toko tadi, anda membuka ransel anda untuk memakai senjata tersebut. Jika opsi View Inventory dipilih, program akan menunjukan senjata apa saja yang dimiliki dan dapat dipakai (jika senjata memiliki passive, tunjukan juga passive tersebut).
Lalu apabila opsi Show Player Stats dipilih saat menggunakan weapon maka Base Damage player akan berubah dan jika memiliki passive, maka akan ada status tambahan yaitu Passive.

```bash
// dungeon.c
void view_inventory(Player *p) {
    char input[10];
    int choice;
    printf(ANSI_BLUE "┌─────────────────────────────────────────────┐\n" ANSI_RESET);
    printf(ANSI_BLUE "│" ANSI_YELLOW "         🎒 HERO'S SATCHEL 🎒               " ANSI_BLUE "│\n" ANSI_RESET);
    printf(ANSI_BLUE "├────┬──────────────────┬──────────────┤\n" ANSI_RESET);
    printf(ANSI_BLUE "│" ANSI_CYAN " ID " ANSI_BLUE "│" ANSI_CYAN " Name             " ANSI_BLUE "│" ANSI_CYAN " Status     " ANSI_BLUE "│\n" ANSI_RESET);
    printf(ANSI_BLUE "├────┼──────────────────┼──────────────┤\n" ANSI_RESET);
    printf(ANSI_BLUE "│" ANSI_CYAN " 1  " ANSI_BLUE "│" ANSI_GREEN " Fists            " ANSI_BLUE "│" ANSI_YELLOW "%-12s" ANSI_BLUE "│\n", 
           strcmp(p->equipped_weapon, "Fists") == 0 ? "EQUIPPED" : "");
    for (int i = 0; i < p->inventory_size; i++) {
        Weapon *w = get_weapon_by_id(p->inventory[i]);
        if (w) {
            printf(ANSI_BLUE "│" ANSI_CYAN " %-2d " ANSI_BLUE "│" ANSI_GREEN " %-16s " ANSI_BLUE "│" ANSI_YELLOW "%-12s" ANSI_BLUE "│\n", 
                   w->id + 1, w->name, strcmp(p->equipped_weapon, w->name) == 0 ? "EQUIPPED" : 
                   strlen(w->passive) > 0 ? w->passive : "");
        }
    }
    printf(ANSI_BLUE "└────┴──────────────────┴──────────────┘\n" ANSI_RESET);
    printf(ANSI_CYAN "➤ Enter number to equip (0 to cancel): " ANSI_RESET);
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%d", &choice);
    if (choice == 0) return;
    if (choice == 1) {
        strcpy(p->equipped_weapon, "Fists");
        p->base_damage = 5;
        printf(ANSI_GREEN "✅ Fists equipped. Ready to brawl!\n" ANSI_RESET);
    } else if (choice >= 2 && choice <= p->inventory_size + 1) {
        Weapon *w = get_weapon_by_id(choice - 1);
        if (w) {
            for (int i = 0; i < p->inventory_size; i++) {
                if (p->inventory[i] == w->id) {
                    strcpy(p->equipped_weapon, w->name);
                    p->base_damage = w->damage;
                    printf(ANSI_GREEN "✅ %s equipped!\n" ANSI_RESET, w->name);
                    return;
                }
            }
            printf(ANSI_RED "⚠ You don't own that weapon!\n" ANSI_RESET);
        } else {
            printf(ANSI_RED "⚠ Invalid weapon.\n" ANSI_RESET);
        }
    } else {
        printf(ANSI_RED "⚠ Invalid choice.\n" ANSI_RESET);
    }
}
```
Fungsi `view_inventory` menampilkan inventaris pemain, termasuk:
- Fists: Selalu tersedia, ID 1, tanpa passive.
- Senjata yang Dimiliki: Ditampilkan dengan ID, nama, dan status (EQUIPPED atau passive jika ada).
Pemain dapat memilih ID senjata untuk mengequip atau 0 untuk membatalkan. Mengequip senjata memperbarui `equipped_weapon` dan `base_damage`. Penanganan kesalahan memastikan validitas input dan kepemilikan senjata.

f. Enemy Encounter -> 
Anda sekarang sudah siap untuk melewati pintu yang seram tadi, disaat anda memasuki pintu tersebut, anda langsung ditemui oleh sebuah musuh yang bukan sebuah manusia. Dengan tekad yang bulat, anda melawan musuh tersebut. Saat opsi Battle Mode dipilih, program akan menunjukan health-bar musuh serta angka yang menunjukan berapa darah musuh tersebut dan menunggu input dengan opsi attack untuk melakukan sebuah serangan dan juga exit untuk keluar dari Battle Mode. Apabila darah musuh berkurang, maka health-bar musuh akan berkurang juga.
Jika darah musuh sudah 0, maka program akan menunjukan rewards berupa berapa banyak gold yang didapatkan lalu akan muncul musuh lagi.

```bash
// dungeon.c
void battle_mode(Player *p) {
    char input[10];
    int enemy_max_hp = 0, enemy_hp = 0;
    int new_enemy = 1;
    printf(ANSI_RED "┌────────────────────────────────────┐\n" ANSI_RESET);
    printf(ANSI_RED "│" ANSI_YELLOW "         ⚡ THE DARK GATE ⚡         " ANSI_RED "│\n" ANSI_RESET);
    printf(ANSI_RED "└────────────────────────────────────┘\n" ANSI_RESET);
    srand(time(NULL));
    while (1) {
        char message[MAX_MESSAGE] = "";
        if (new_enemy) {
            enemy_max_hp = rand() % 151 + 50;
            enemy_hp = enemy_max_hp;
            printf(ANSI_YELLOW "👹 A shadowy beast appears!\n" ANSI_RESET);
            printf(ANSI_RED "👾 Enemy status:\n" ANSI_RESET);
            print_health_bar(enemy_hp, enemy_max_hp);
            new_enemy = 0;
        }
        printf(ANSI_CYAN "\n➤ Type '" ANSI_GREEN "attack" ANSI_CYAN "' or '" ANSI_RED "exit" ANSI_CYAN "': " ANSI_RESET);
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;
        if (strcmp(input, "exit") == 0) {
            printf(ANSI_YELLOW "🏃 You fled into the shadows.\n" ANSI_RESET);
            return;
        } else if (strcmp(input, "attack") == 0) {
            int damage = p->base_damage + (rand() % 6 + 1);
            int is_crit = (rand() % 100 < 10);
            if (strcmp(p->equipped_weapon, "Dragon Claws") == 0) {
                is_crit = (rand() % 100 < 30);
            } else if (strcmp(p->equipped_weapon, "Staff of Light") == 0) {
                if (rand() % 100 < 10) {
                    damage = enemy_hp;
                    snprintf(message, sizeof(message), ANSI_PURPLE "✨ STAFF OF LIGHT INSTANT KILL!\n" ANSI_RESET);
                }
            }
            if (is_crit && strlen(message) == 0) {
                damage *= 2;
                snprintf(message, sizeof(message), ANSI_RED "💥 CRITICAL HIT! Dealt %d damage!\n" ANSI_RESET, damage);
            } else if (strlen(message) == 0) {
                snprintf(message, sizeof(message), ANSI_GREEN "⚔ Dealt %d damage!\n" ANSI_RESET, damage);
            }
            enemy_hp -= damage;
            printf("%s", message);
            if (enemy_hp <= 0) {
                int gold_reward = rand() % 101 + 50;
                p->gold += gold_reward;
                p->kills++;
                printf(ANSI_YELLOW "🎉 Enemy defeated! Earned %d gold!\n" ANSI_RESET, gold_reward);
                new_enemy = 1;
            } else {
                printf(ANSI_RED "👾 Enemy status:\n" ANSI_RESET);
                print_health_bar(enemy_hp, enemy_max_hp);
            }
        } else {
            printf(ANSI_RED "⚠ Invalid command.\n" ANSI_RESET);
        }
    }
}
```
Fungsi `battle_mode` mengatur sistem pertempuran. Musuh baru muncul dengan HP acak 50–200. Health-bar ditampilkan menggunakan blok Unicode, berubah merah saat HP rendah. Pemain dapat:
- attack: Menyerang dengan damage `base_damage + (rand() % 6 + 1)`, dengan 10% peluang critical (damage x2), 30% untuk Dragon Claws. Staff of Light memiliki 10% peluang instan kill.
- exit: Keluar dari mode pertempuran.
Saat HP musuh 0, pemain mendapat 50–150 gold, jumlah kill bertambah, dan musuh baru muncul. Health-bar diperbarui setiap serangan, mencerminkan HP musuh.

g. Other Battle Logic -> 
1. Health & Rewards: 
Untuk darah musuh, seberapa banyak darah yang mereka punya dibuat secara random, contoh: 50-200 HP. Lakukan hal yang sama untuk rewards. 

```bash
// dungeon.c
void battle_mode(Player *p) {
    char input[10];
    int enemy_max_hp = 0, enemy_hp = 0;
    int new_enemy = 1;
    printf(ANSI_RED "┌────────────────────────────────────┐\n" ANSI_RESET);
    printf(ANSI_RED "│" ANSI_YELLOW "         ⚡ THE DARK GATE ⚡         " ANSI_RED "│\n" ANSI_RESET);
    printf(ANSI_RED "└────────────────────────────────────┘\n" ANSI_RESET);
    srand(time(NULL));
    while (1) {
        char message[MAX_MESSAGE] = "";
        if (new_enemy) {
            enemy_max_hp = rand() % 151 + 50; // HP musuh acak 50–200
            enemy_hp = enemy_max_hp;
            printf(ANSI_YELLOW "👹 A shadowy beast appears!\n" ANSI_RESET);
            printf(ANSI_RED "👾 Enemy status:\n" ANSI_RESET);
            print_health_bar(enemy_hp, enemy_max_hp);
            new_enemy = 0;
        }
        printf(ANSI_CYAN "\n➤ Type '" ANSI_GREEN "attack" ANSI_CYAN "' or '" ANSI_RED "exit" ANSI_CYAN "': " ANSI_RESET);
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;
        if (strcmp(input, "exit") == 0) {
            printf(ANSI_YELLOW "🏃 You fled into the shadows.\n" ANSI_RESET);
            return;
        } else if (strcmp(input, "attack") == 0) {
            int damage = p->base_damage + (rand() % 6 + 1); // Damage acak
            int is_crit = (rand() % 100 < 10);
            if (strcmp(p->equipped_weapon, "Dragon Claws") == 0) {
                is_crit = (rand() % 100 < 30); // 30% crit untuk Dragon Claws
            } else if (strcmp(p->equipped_weapon, "Staff of Light") == 0) {
                if (rand() % 100 < 10) { // 10% instan kill
                    damage = enemy_hp;
                    snprintf(message, sizeof(message), ANSI_PURPLE "✨ STAFF OF LIGHT INSTANT KILL!\n" ANSI_RESET);
                }
            }
            if (is_crit && strlen(message) == 0) {
                damage *= 2;
                snprintf(message, sizeof(message), ANSI_RED "💥 CRITICAL HIT! Dealt %d damage!\n" ANSI_RESET, damage);
            } else if (strlen(message) == 0) {
                snprintf(message, sizeof(message), ANSI_GREEN "⚔ Dealt %d damage!\n" ANSI_RESET, damage);
            }
            enemy_hp -= damage;
            printf("%s", message);
            if (enemy_hp <= 0) {
                int gold_reward = rand() % 101 + 50; // Reward acak 50–150
                p->gold += gold_reward;
                p->kills++;
                printf(ANSI_YELLOW "🎉 Enemy defeated! Earned %d gold!\n" ANSI_RESET, gold_reward);
                new_enemy = 1;
            } else {
                printf(ANSI_RED "👾 Enemy status:\n" ANSI_RESET);
                print_health_bar(enemy_hp, enemy_max_hp);
            }
        } else {
            printf(ANSI_RED "⚠ Invalid command.\n" ANSI_RESET);
        }
    }
}
```
  - HP Musuh: HP musuh dihasilkan secara acak dengan `rand() % 151 + 50`, memberikan rentang 50–200 HP, sesuai permintaan.
  - Rewards: Setelah mengalahkan musuh, pemain mendapat gold acak dengan `rand() % 101 + 50`, memberikan rentang 50–150 gold, juga sesuai permintaan.
  - Health Bar: Fungsi `print_health_bar` menampilkan status HP musuh secara visual menggunakan blok Unicode, dengan warna merah saat HP rendah.

2. Damage Equation: 
Untuk damage, gunakan base damage sebagai kerangka awal dan tambahkan rumus damage apapun (dibebaskan, yang pasti perlu random number agar hasil damage bervariasi). Lalu buatlah logic agar setiap serangan memiliki kesempatan untuk Critical yang membuat damage anda 2x lebih besar.

3. Passive: 
Jika senjata yang dipakai memiliki Passive setiap kali passive tersebut menyala, maka tunjukan bahwa passive tersebut aktif.

```bash
// dungeon.c
void battle_mode(Player *p) {
    char input[10];
    int enemy_max_hp = 0, enemy_hp = 0;
    int new_enemy = 1;
    printf(ANSI_RED "┌────────────────────────────────────┐\n" ANSI_RESET);
    printf(ANSI_RED "│" ANSI_YELLOW "         ⚡ THE DARK GATE ⚡         " ANSI_RED "│\n" ANSI_RESET);
    printf(ANSI_RED "└────────────────────────────────────┘\n" ANSI_RESET);
    srand(time(NULL));
    while (1) {
        char message[MAX_MESSAGE] = "";
        if (new_enemy) {
            enemy_max_hp = rand() % 151 + 50; // HP musuh acak 50–200
            enemy_hp = enemy_max_hp;
            printf(ANSI_YELLOW "👹 A shadowy beast appears!\n" ANSI_RESET);
            printf(ANSI_RED "👾 Enemy status:\n" ANSI_RESET);
            print_health_bar(enemy_hp, enemy_max_hp);
            new_enemy = 0;
        }
        printf(ANSI_CYAN "\n➤ Type '" ANSI_GREEN "attack" ANSI_CYAN "' or '" ANSI_RED "exit" ANSI_CYAN "': " ANSI_RESET);
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;
        if (strcmp(input, "exit") == 0) {
            printf(ANSI_YELLOW "🏃 You fled into the shadows.\n" ANSI_RESET);
            return;
        } else if (strcmp(input, "attack") == 0) {
            // Rumus damage: base_damage + komponen acak (1–6)
            int damage = p->base_damage + (rand() % 6 + 1);
            int is_crit = (rand() % 100 < 10); // 10% peluang critical default
            char passive_message[MAX_MESSAGE] = "";

            // Periksa passive senjata
            if (strcmp(p->equipped_weapon, "Dragon Claws") == 0) {
                is_crit = (rand() % 100 < 30); // 30% peluang critical
                if (is_crit) {
                    snprintf(passive_message, sizeof(passive_message), 
                             ANSI_PURPLE "✨ Dragon Claws Passive: Critical Chance Activated!\n" ANSI_RESET);
                }
            } else if (strcmp(p->equipped_weapon, "Staff of Light") == 0) {
                if (rand() % 100 < 10) { // 10% peluang instan kill
                    damage = enemy_hp;
                    snprintf(passive_message, sizeof(passive_message), 
                             ANSI_PURPLE "✨ Staff of Light Passive: Instant Kill Activated!\n" ANSI_RESET);
                }
            }

            // Terapkan critical hit
            if (is_crit && strlen(passive_message) == 0) {
                damage *= 2;
                snprintf(message, sizeof(message), 
                         ANSI_RED "💥 CRITICAL HIT! Dealt %d damage!\n" ANSI_RESET, damage);
            } else if (strlen(passive_message) > 0) {
                snprintf(message, sizeof(message), 
                         "%sDealt %d damage!\n", passive_message, damage);
            } else {
                snprintf(message, sizeof(message), 
                         ANSI_GREEN "⚔ Dealt %d damage!\n" ANSI_RESET, damage);
            }

            enemy_hp -= damage;
            printf("%s", message);
            if (enemy_hp <= 0) {
                int gold_reward = rand() % 101 + 50; // Reward acak 50–150
                p->gold += gold_reward;
                p->kills++;
                printf(ANSI_YELLOW "🎉 Enemy defeated! Earned %d gold!\n" ANSI_RESET, gold_reward);
                new_enemy = 1;
            } else {
                printf(ANSI_RED "👾 Enemy status:\n" ANSI_RESET);
                print_health_bar(enemy_hp, enemy_max_hp);
            }
        } else {
            printf(ANSI_RED "⚠ Invalid command. Please type 'attack' or 'exit'.\n" ANSI_RESET);
        }
    }
}
```
  - Rumus Damage: Damage dihitung sebagai `base_damage + (rand() % 6 + 1)`, di mana `base_damage` berasal dari senjata yang digunakan (misalnya, 5 untuk Fists, 50 untuk Dragon Claws), dan komponen acak (1–6) menambahkan variasi.
  - Critical Hit: Setiap serangan memiliki peluang 10% untuk critical hit (`rand() % 100 < 10`), menggandakan damage (`damage *= 2`). Untuk Dragon Claws, peluang critical meningkat menjadi 30% karena passive-nya.
  - Passive Activation:
    - Dragon Claws: Jika critical hit terjadi, pesan "Dragon Claws Passive: Critical Chance Activated!" ditampilkan.
    - Staff of Light: Jika instan kill (10% peluang) terjadi, pesan "Staff of Light Passive: Instant Kill Activated!" ditampilkan, dan damage disetel ke HP musuh untuk membunuh langsung.
  - Output: Pesan damage mencakup informasi critical atau passive untuk kejelasan. Jika tidak ada critical atau passive, pesan standar menunjukkan damage yang diberikan.

h. Error Handling
Berikan error handling untuk opsi-opsi yang tidak ada.

    1. Menu Utama (Error Handling untuk Pilihan Tidak Valid)
        - Input Tidak Valid: Fungsi sscanf memeriksa apakah input adalah angka. Jika bukan (misalnya, huruf atau simbol), pesan kesalahan ditampilkan: "Invalid input. Please enter a number between 1 and 5."
        - Pilihan di Luar Rentang: Jika angka di luar 1–5, pernyataan default dalam switch menampilkan pesan: "The dungeon whispers: 'Choose a valid path (1–5)!'" Ini memastikan pengguna hanya memilih opsi yang tersedia.
```bash
// dungeon.c
void main_menu(Player *p) {
    char input[10];
    int choice;
    while (1) {
        printf(ANSI_GREEN "┌────────────────────────────────────┐\n" ANSI_RESET);
        printf(ANSI_GREEN "│" ANSI_YELLOW "       🏰 LOST DUNGEON 🏰           " ANSI_GREEN "│\n" ANSI_RESET);
        printf(ANSI_GREEN "├────────────────────────────────────┤\n" ANSI_RESET);
        printf(ANSI_GREEN "│ " ANSI_CYAN "[1] Hero's Journal (Stats)   📜" ANSI_RESET " " ANSI_GREEN "│\n" ANSI_RESET);
        printf(ANSI_GREEN "│ " ANSI_CYAN "[2] Mysterious Armory (Shop) 🛒" ANSI_RESET " " ANSI_GREEN "│\n" ANSI_RESET);
        printf(ANSI_GREEN "│ " ANSI_CYAN "[3] Hero's Satchel (Inventory)🎒" ANSI_RESET " " ANSI_GREEN "│\n" ANSI_RESET);
        printf(ANSI_GREEN "│ " ANSI_CYAN "[4] The Dark Gate (Battle)   ⚔" ANSI_RESET " " ANSI_GREEN "│\n" ANSI_RESET);
        printf(ANSI_GREEN "│ " ANSI_CYAN "[5] Flee the Dungeon (Exit)  🏃" ANSI_RESET " " ANSI_GREEN "│\n" ANSI_RESET);
        printf(ANSI_GREEN "└────────────────────────────────────┘\n" ANSI_RESET);
        printf(ANSI_YELLOW "➤ Choose your path: " ANSI_RESET);
        fgets(input, sizeof(input), stdin);
        if (sscanf(input, "%d", &choice) != 1) {
            printf(ANSI_RED "⚠ Invalid input. Please enter a number between 1 and 5.\n" ANSI_RESET);
            continue;
        }
        switch (choice) {
            case 1: show_player_stats(p); break;
            case 2: display_shop(p); break;
            case 3: view_inventory(p); break;
            case 4: battle_mode(p); break;
            case 5:
                printf(ANSI_YELLOW "🏃 You escape the dungeon's grasp... for now.\n" ANSI_RESET);
                return;
            default:
                printf(ANSI_RED "⚠ The dungeon whispers: 'Choose a valid path (1–5)!'\n" ANSI_RESET);
                break;
        }
    }
}
```

    2. Toko Senjata (Error Handling untuk Pembelian)
        - Input Tidak Valid: Jika input bukan angka, sscanf gagal, dan pesan kesalahan ditampilkan: "Invalid input. Please enter a number (0–5)."
        - Pilihan Tidak Valid: Fungsi buy_weapon mengembalikan kode kesalahan: -1: ID senjata tidak valid (bukan 1–5), -2: Gold tidak cukup, 3: Senjata sudah dimiliki., dan Lainnya: Inventaris penuh.
        - Setiap kasus menampilkan pesan spesifik untuk membantu pengguna memahami kesalahan.
```bash
// shop.c
void display_shop(Player *p) {
    char input[10];
    int choice;
    printf(ANSI_PURPLE "┌─────────────────────────────────────────────┐\n" ANSI_RESET);
    printf(ANSI_PURPLE "│" ANSI_YELLOW "         🛒 MYSTERIOUS ARMORY 🛒             " ANSI_PURPLE "│\n" ANSI_RESET);
    printf(ANSI_PURPLE "├────┬──────────────────┬──────┬──────┬────────┤\n" ANSI_RESET);
    printf(ANSI_PURPLE "│" ANSI_CYAN " ID " ANSI_PURPLE "│" ANSI_CYAN " Name             " ANSI_PURPLE "│" ANSI_CYAN " Gold " ANSI_PURPLE "│" ANSI_CYAN " Dmg  " ANSI_PURPLE "│" ANSI_CYAN " Passive" ANSI_PURPLE "│\n" ANSI_RESET);
    printf(ANSI_PURPLE "├────┼──────────────────┼──────┼──────┼────────┤\n" ANSI_RESET);
    for (int i = 0; i < WEAPON_COUNT; i++) {
        Weapon *w = &shop_weapons[i];
        printf(ANSI_PURPLE "│" ANSI_CYAN " %-2d " ANSI_PURPLE "│" ANSI_GREEN " %-16s " ANSI_PURPLE "│" ANSI_YELLOW " %-4d " ANSI_PURPLE "│" ANSI_RED " %-4d " ANSI_PURPLE "│" ANSI_BLUE " %-6s " ANSI_PURPLE "│\n", 
               w->id, w->name, w->price, w->damage, w->passive);
    }
    printf(ANSI_PURPLE "└────┴──────────────────┴──────┴──────┴────────┘\n" ANSI_RESET);
    printf(ANSI_YELLOW "💬 The merchant grins: 'Choose wisely, adventurer!'\n" ANSI_RESET);
    printf(ANSI_CYAN "➤ Enter weapon number to buy (0 to leave): " ANSI_RESET);
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%d", &choice) != 1) {
        printf(ANSI_RED "⚠ Invalid input. Please enter a number (0–5).\n" ANSI_RESET);
        return;
    }
    if (choice == 0) {
        printf(ANSI_YELLOW "💬 The merchant nods: 'Come back anytime!'\n" ANSI_RESET);
        return;
    }
    int result = buy_weapon(p, choice);
    if (result == 0) {
        printf(ANSI_GREEN "✅ Successfully bought %s!\n" ANSI_RESET, p->equipped_weapon);
    } else if (result == -1) {
        printf(ANSI_RED "⚠ Invalid weapon ID. Choose a number between 1 and 5.\n" ANSI_RESET);
    } else if (result == -2) {
        printf(ANSI_RED "⚠ The merchant laughs: 'Not enough gold, hero!'\n" ANSI_RESET);
    } else if (result == -3) {
        printf(ANSI_RED "⚠ You already own this weapon!\n" ANSI_RESET);
    } else {
        printf(ANSI_RED "⚠ Inventory full! Cannot carry more weapons.\n" ANSI_RESET);
    }
}
```

    3. Inventaris (Error Handling untuk Equip)
        - Input Tidak Valid: Jika input bukan angka, pesan kesalahan ditampilkan: "Invalid input. Please enter a number."
        - Pilihan di Luar Rentang: Jika pilihan di luar 0 hingga inventory_size + 1, pesan ditampilkan dengan rentang yang valid.
        - Senjata Tidak Dimiliki: Jika ID senjata tidak ada di inventaris, pesan ditampilkan: "You don't own that weapon!" atau "Invalid weapon ID."
```bash
// dungeon.c
void view_inventory(Player *p) {
    char input[10];
    int choice;
    printf(ANSI_BLUE "┌─────────────────────────────────────────────┐\n" ANSI_RESET);
    printf(ANSI_BLUE "│" ANSI_YELLOW "         🎒 HERO'S SATCHEL 🎒               " ANSI_BLUE "│\n" ANSI_RESET);
    printf(ANSI_BLUE "├────┬──────────────────┬──────────────┤\n" ANSI_RESET);
    printf(ANSI_BLUE "│" ANSI_CYAN " ID " ANSI_BLUE "│" ANSI_CYAN " Name             " ANSI_BLUE "│" ANSI_CYAN " Status     " ANSI_BLUE "│\n" ANSI_RESET);
    printf(ANSI_BLUE "├────┼──────────────────┼──────────────┤\n" ANSI_RESET);
    printf(ANSI_BLUE "│" ANSI_CYAN " 1  " ANSI_BLUE "│" ANSI_GREEN " Fists            " ANSI_BLUE "│" ANSI_YELLOW "%-12s" ANSI_BLUE "│\n", 
           strcmp(p->equipped_weapon, "Fists") == 0 ? "EQUIPPED" : "");
    for (int i = 0; i < p->inventory_size; i++) {
        Weapon *w = get_weapon_by_id(p->inventory[i]);
        if (w) {
            printf(ANSI_BLUE "│" ANSI_CYAN " %-2d " ANSI_BLUE "│" ANSI_GREEN " %-16s " ANSI_BLUE "│" ANSI_YELLOW "%-12s" ANSI_BLUE "│\n", 
                   w->id + 1, w->name, strcmp(p->equipped_weapon, w->name) == 0 ? "EQUIPPED" : 
                   strlen(w->passive) > 0 ? w->passive : "");
        }
    }
    printf(ANSI_BLUE "└────┴──────────────────┴──────────────┘\n" ANSI_RESET);
    printf(ANSI_CYAN "➤ Enter number to equip (0 to cancel): " ANSI_RESET);
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%d", &choice) != 1) {
        printf(ANSI_RED "⚠ Invalid input. Please enter a number.\n" ANSI_RESET);
        return;
    }
    if (choice == 0) return;
    if (choice == 1) {
        strcpy(p->equipped_weapon, "Fists");
        p->base_damage = 5;
        printf(ANSI_GREEN "✅ Fists equipped. Ready to brawl!\n" ANSI_RESET);
    } else if (choice >= 2 && choice <= p->inventory_size + 1) {
        Weapon *w = get_weapon_by_id(choice - 1);
        if (w) {
            for (int i = 0; i < p->inventory_size; i++) {
                if (p->inventory[i] == w->id) {
                    strcpy(p->equipped_weapon, w->name);
                    p->base_damage = w->damage;
                    printf(ANSI_GREEN "✅ %s equipped!\n" ANSI_RESET, w->name);
                    return;
                }
            }
            printf(ANSI_RED "⚠ You don't own that weapon!\n" ANSI_RESET);
        } else {
            printf(ANSI_RED "⚠ Invalid weapon ID. Choose a valid number.\n" ANSI_RESET);
        }
    } else {
        printf(ANSI_RED "⚠ Invalid choice. Choose a number between 0 and %d.\n" ANSI_RESET, 
               p->inventory_size + 1);
    }
}
```

    4. Mode Pertempuran (Error Handling untuk Perintah)
        - Perintah Tidak Valid: Jika pengguna memasukkan selain "attack" atau "exit" (misalnya, "jump" atau kosong), pesan ditampilkan: "Invalid command. Please type 'attack' or 'exit'." Ini memastikan hanya perintah yang valid yang diproses.
```bash
if (strcmp(input, "exit") == 0) {
    printf(ANSI_YELLOW "🏃 You fled into the shadows.\n" ANSI_RESET);
    return;
} else if (strcmp(input, "attack") == 0) {
    // Logika serangan...
} else {
    printf(ANSI_RED "⚠ Invalid command. Please type 'attack' or 'exit'.\n" ANSI_RESET);
}
```


## Soal 4
[Author: Rafa / kookoon]

Di dunia yang sedang kacau ini, muncul seorang hunter lemah bernama Sung Jin Woo😈yang bahkan tidak bisa mengangkat sebuah galon🛢️. Suatu hari, hunter ini tertabrak sebuah truk dan meninggal di tempat. Dia pun marah akan dirinya yang lemah dan berharap mendapat kesempatan kedua. Beruntungnya, Sung Jin Woo pun mengalami reinkarnasi dan sekarang bekerja sebagai seorang admin. Uniknya, pekerjaan ini mempunyai sebuah sistem yang bisa melakukan tracking pada seluruh aktivitas dan keadaan seseorang. Sayangnya, model yang diberikan oleh Bos-nya sudah sangat tua sehingga program tersebut harus dimodifikasi agar tidak ketinggalan zaman, dengan spesifikasi:

a. Agar hunter lain tidak bingung, Sung Jin Woo memutuskan untuk membuat dua file, yaitu system.c dan hunter.c. Sung Jin Woo mendapat peringatan bahwa system.c merupakan shared memory utama yang mengelola shared memory hunter-hunter dari hunter.c. Untuk mempermudahkan pekerjaannya, Sung Jin Woo mendapat sebuah clue yang dapat membuat pekerjaannya menjadi lebih mudah dan efisien. NOTE : hunter bisa dijalankan ketika sistem sudah dijalankan.

```bash
// system.c
int shmid;
void cleanup_shared_memory() {
    printf("Cleaning up shared memory...\n");
    struct SystemData *data = shmat(shmid, NULL, 0);
    if (data != (void*)-1) {
        shmdt(data);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Failed to delete shared memory");
        printf("shmid: %d, errno: %d\n", shmid, errno);
    } else {
        printf("✓ Shared memory deleted (ID: %d)\n", shmid);
    }
}
int main() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    srand(time(NULL));
    key_t key = get_system_key();
    shmid = shmget(key, sizeof(struct SystemData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    printf(GREEN "✓ " RESET "Shared memory created with ID: %d\n", shmid);
    struct SystemData *data = shmat(shmid, NULL, 0);
    if (data == (void *) -1) {
        printf(RED "ERROR: Could not attach to memory\n" RESET);
        perror("shmat");
        exit(1);
    }
    initialize_memory(data);
    // Menu admin...
}
```
- Sistem terbagi menjadi `system.c` (antarmuka admin) dan `hunter.c` (antarmuka hunter). `system.c` membuat shared memory dengan `shmget` dan menginisialisasinya dengan `initialize_memory`. `hunter.c` terhubung ke shared memory yang sama. Fungsi `cleanup_shared_memory` menghapus shared memory saat sistem dimatikan, mencegah kebocoran data. Ini memastikan `hunter.c` hanya berfungsi setelah `system.c` menginisialisasi shared memory.


b. Untuk memastikan keteraturan sistem, Sung Jin Woo memutuskan untuk membuat fitur registrasi dan login di program hunter. Setiap hunter akan memiliki key unik dan stats awal (Level=1, EXP=0, ATK=10, HP=100, DEF=5). Data hunter disimpan dalam shared memory tersendiri yang terhubung dengan sistem.

```bash
// hunter.c
struct Hunter* register_hunter(struct SystemData *data) {
    char uname[50];
    printf(" Buat username baru: ");
    scanf("%s", uname);
    getchar();
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, uname) == 0) {
            printf(RED "❌ Username sudah digunakan.\n" RESET);
            sleep(2);
            return NULL;
        }
    }
    if (data->num_hunters >= MAX_HUNTERS) {
        printf(RED "❌ Server penuh. Tidak bisa mendaftar lagi.\n" RESET);
        sleep(2);
        return NULL;
    }
    struct Hunter *new_hunter = &data->hunters[data->num_hunters];
    strcpy(new_hunter->username, uname);
    new_hunter->level = 1;
    new_hunter->exp = 0;
    new_hunter->atk = 10;
    new_hunter->hp = 100;
    new_hunter->def = 5;
    new_hunter->banned = 0;
    new_hunter->shm_key = get_system_key();
    data->num_hunters++;
    printf(GREEN "✓ Akun berhasil dibuat! Selamat datang, %s!\n" RESET, uname);
    sleep(2);
    return new_hunter;
}
```
Fungsi `register_hunter` membuat akun baru dengan memeriksa keunikan username dan batas hunter (`MAX_HUNTERS`). Hunter baru diinisialisasi dengan stats awal dan disimpan di shared memory. Fungsi `login` memverifikasi username dan memeriksa status banned. Kedua fungsi memberikan pesan kesalahan untuk input tidak valid, memastikan data konsisten di shared memory.

c. Agar dapat memantau perkembangan para hunter dengan mudah, Sung Jin Woo menambahkan fitur di sistem yang dapat menampilkan informasi semua hunter yang terdaftar, termasuk nama hunter, level, exp, atk, hp, def, dan status (banned atau tidak). Ini membuat dia dapat melihat siapa hunter terkuat dan siapa yang mungkin melakukan kecurangan.

```bash
// system.c
void show_all_hunters(struct SystemData *data) {
    printf(CLEAR_SCREEN);
    print_header(CYAN "REGISTERED HUNTERS LIST" YELLOW);
    if (data->num_hunters == 0) {
        printf(" " RED "No hunters registered yet..." RESET "                             \n");
    } else {
        for (int i = 0; i < data->num_hunters; i++) {
            char status[20];
            sprintf(status, "%s", data->hunters[i].banned ? RED "BANNED" RESET : GREEN "ACTIVE" RESET);
            printf(" " CYAN "[%d]" RESET " %-20s | " YELLOW "Lv:" RESET "%2d | " YELLOW "EXP:" RESET "%3d | " 
                   RED "ATK:" RESET "%3d | " GREEN "HP:" RESET "%3d | " BLUE "DEF:" RESET "%2d | %-10s \n", 
                i + 1, data->hunters[i].username, data->hunters[i].level, data->hunters[i].exp,
                data->hunters[i].atk, data->hunters[i].hp, data->hunters[i].def, status);
        }
    }
    print_footer();
}
```
Fungsi `show_all_hunters` menampilkan daftar hunter dengan detail stats dan status. Jika tidak ada hunter, pesan ditampilkan. Fitur ini memungkinkan admin memantau perkembangan hunter dan mendeteksi kecurangan berdasarkan stats tinggi atau status banned.

d. Setelah beberapa hari bekerja, Sung Jin Woo menyadari bahwa para hunter membutuhkan tempat untuk berlatih dan memperoleh pengalaman. Ia memutuskan untuk membuat fitur unik dalam sistem yang dapat menghasilkan dungeon secara random dengan nama, level minimal hunter, dan stat rewards dengan nilai:
- 🏆Level Minimal : 1 - 5
- ⚔️ATK : 100 - 150 Poin
- ❤️HP  : 50 - 100 Poin
- 🛡️DEF : 25 - 50 Poin
- 🌟EXP : 150 - 300 Poin
Setiap dungeon akan disimpan dalam shared memory sendiri yang berbeda dan dapat diakses oleh hunter.

```bash
// system.c
void generate_dungeon(struct SystemData *data) {
    printf(CLEAR_SCREEN);
    print_header(MAGENTA "GENERATE NEW DUNGEON" YELLOW);
    char *names[] = {"Demon Castle", "Shadow Temple", "Ant Nest", "Dragon Lair", "Cursed Forest",
                    "Haunted Mansion", "Goblin Cave", "Mystic Shrine", "Pirate Bay", "Underworld"};
    char *difficulty[] = {"Easy", "Normal", "Hard", "Expert", "Master"};
    int idx = rand() % 10;
    int diff = rand() % 5;
    if (data->num_dungeons >= MAX_DUNGEONS) {
        printf(" " RED "No more space for dungeons! Maximum reached (%d)." RESET "         \n", MAX_DUNGEONS);
        print_footer();
        return;
    }
    struct Dungeon *new_dungeon = &data->dungeons[data->num_dungeons];
    char full_name[50];
    snprintf(full_name, sizeof(full_name), "%s (%s)", names[idx], difficulty[diff]);
    strncpy(new_dungeon->name, full_name, sizeof(new_dungeon->name) - 1);
    new_dungeon->min_level = diff + 1;
    new_dungeon->exp = (rand() % 101) + 100 + (diff * 50);
    new_dungeon->atk = (rand() % 31) + 10 + (diff * 20);
    new_dungeon->hp = (rand() % 51) + 50 + (diff * 20);
    new_dungeon->def = (rand() % 16) + 10 + (diff * 5);
    new_dungeon->shm_key = get_system_key();
    data->num_dungeons++;
    printf(" " GREEN "⚔️  Dungeon Created Successfully! ⚔️" RESET "                       \n");
    printf(" " CYAN "Name:      " RESET "%s                              \n", new_dungeon->name);
    print_footer();
}
```
Fungsi `generate_dungeon` membuat dungeon acak dengan nama dan difficulty acak. Stats dihitung berdasarkan rentang yang ditentukan, disimpan di shared memory untuk akses hunter. Batas `MAX_DUNGEONS` mencegah kelebihan dungeon, dengan pesan kesalahan jika penuh.

e. Untuk memudahkan admin dalam memantau dungeon yang muncul, Sung Jin Woo menambahkan fitur yang menampilkan informasi detail semua dungeon. Fitur ini menampilkan daftar lengkap dungeon beserta nama, level minimum, reward (EXP, ATK, HP, DEF), dan key unik untuk masing-masing dungeon.

```bash
// system.c
void show_all_dungeons(struct SystemData *data) {
    printf(CLEAR_SCREEN);
    print_header(MAGENTA "                                    AVAILABLE DUNGEONS" YELLOW);
    if (data->num_dungeons == 0) {
        printf(" " RED "No dungeons available yet..." RESET "\n");
        print_footer();
        return;
    }
    for (int i = 0; i < data->num_dungeons; i++) {
        printf(" " MAGENTA "[%d]" RESET " %-25s | Min Lv:%2d | EXP:%3d | ATK:+%2d | HP:+%3d | DEF:+%2d\n",
            i + 1, data->dungeons[i].name, data->dungeons[i].min_level, data->dungeons[i].exp,
            data->dungeons[i].atk, data->dungeons[i].hp, data->dungeons[i].def);
    }
    char opt;
    do {
        printf("\n  Do you want to delete a dungeon? (y/n): ");
        scanf(" %c", &opt);
        getchar();
        if (opt == 'y' || opt == 'Y') {
            int del_idx;
            printf("Enter the dungeon number to delete: ");
            scanf("%d", &del_idx);
            getchar();
            if (del_idx < 1 || del_idx > data->num_dungeons) {
                printf(RED "Invalid dungeon number!\n" RESET);
            } else {
                del_idx--;
                printf(YELLOW "Deleting dungeon: %s...\n" RESET, data->dungeons[del_idx].name);
                for (int i = del_idx; i < data->num_dungeons - 1; i++) {
                    data->dungeons[i] = data->dungeons[i + 1];
                }
                data->num_dungeons--;
                printf(GREEN "Dungeon [%d] has been deleted.\n" RESET, del_idx + 1);
            }
        }
    } while ((opt == 'y' || opt == 'Y') && data->num_dungeons > 0);
    print_footer();
}
```
Fungsi `show_all_dungeons` menampilkan daftar dungeon dengan detail dan memungkinkan admin menghapus dungeon dengan memilih nomor. Penanganan kesalahan memastikan nomor valid, dan perubahan disimpan di shared memory.

f. Pada saat yang sama, dungeon yang dibuat oleh sistem juga harus dapat diakses oleh hunter. Sung Jin Woo menambahkan fitur yang menampilkan semua dungeon yang tersedia sesuai dengan level hunter. Disini, hunter hanya dapat menampilkan dungeon dengan level minimum yang sesuai dengan level mereka.

```bash
// hunter.c
void show_dungeons(struct Hunter *hunter, struct SystemData *data) {
    printf(CLEAR_SCREEN);
    print_header(MAGENTA "AVAILABLE DUNGEONS" YELLOW);
    int found = 0;
    for (int i = 0; i < data->num_dungeons; i++) {
        if (hunter->level >= data->dungeons[i].min_level) {
            found++;
            printf(" " CYAN "[%d]" RESET " %-20s | " YELLOW "Min Lv:" RESET "%d | " MAGENTA "EXP:" RESET "%3d | " 
                   RED "ATK:" RESET "%3d | " GREEN "HP:" RESET "%3d | " BLUE "DEF:" RESET "%2d \n",
                i + 1, data->dungeons[i].name, data->dungeons[i].min_level, data->dungeons[i].exp,
                data->dungeons[i].atk, data->dungeons[i].hp, data->dungeons[i].def);
        }
    }
    if (!found) {
        printf(" " RED "No available dungeon for your level." RESET "                     \n");
        printf(" " YELLOW "Try to level up to access more dungeons!" RESET "                  \n");
    }
    print_footer();
}
```
Fungsi `show_dungeons` hanya menampilkan dungeon yang sesuai dengan level hunter (`hunter->level >= dungeon->min_level`). Jika tidak ada dungeon yang tersedia, pesan ditampilkan untuk menyarankan naik level. Data diambil dari shared memory untuk akses real-time.

g. Setelah melihat beberapa hunter yang terlalu kuat, Sung Jin Woo memutuskan untuk menambahkan fitur untuk menguasai dungeon. Ketika hunter berhasil menaklukan sebuah dungeon, dungeon tersebut akan menghilang dari sistem dan hunter akan mendapatkan stat rewards dari dungeon. Jika exp hunter mencapai 500, mereka akan naik level dan exp kembali ke 0.

```bash
// hunter.c
void raid_dungeon(struct Hunter *hunter, struct SystemData *data) {
    printf(CLEAR_SCREEN);
    print_header(GREEN "RAID DUNGEON" YELLOW);
    int found = 0;
    for (int i = 0; i < data->num_dungeons; i++) {
        if (hunter->level >= data->dungeons[i].min_level) {
            found++;
            printf(" " CYAN "[%d]" RESET " %-20s | " YELLOW "Min Lv:" RESET "%d | " MAGENTA "EXP:" RESET "%3d | " 
                   RED "ATK:" RESET "%3d | " GREEN "HP:" RESET "%3d | " BLUE "DEF:" RESET "%2d \n",
                i + 1, data->dungeons[i].name, data->dungeons[i].min_level, data->dungeons[i].exp,
                data->dungeons[i].atk, data->dungeons[i].hp, data->dungeons[i].def);
        }
    }
    if (!found) {
        printf(" " RED "No available dungeon for your level." RESET "                     \n");
        print_footer();
        return;
    }
    printf(" Choose dungeon to raid (number): ");
    int id;
    scanf("%d", &id);
    getchar();
    id--;
    if (id < 0 || id >= data->num_dungeons || hunter->level < data->dungeons[id].min_level) {
        printf(" " RED "Invalid dungeon selected. Please try again." RESET "               \n");
        print_footer();
        return;
    }
    hunter->exp += data->dungeons[id].exp;
    hunter->atk += data->dungeons[id].atk;
    hunter->hp += data->dungeons[id].hp;
    hunter->def += data->dungeons[id].def;
    bool level_up = false;
    if (hunter->exp >= 500) {
        hunter->level++;
        hunter->exp = 0;
        level_up = true;
    }
    for (int i = id; i < data->num_dungeons - 1; i++) {
        data->dungeons[i] = data->dungeons[i + 1];
    }
    data->num_dungeons--;
    printf(" " GREEN "⚔️  DUNGEON CLEARED SUCCESSFULLY! ⚔️" RESET "                       \n");
    print_footer();
}
```
Fungsi `raid_dungeon` memungkinkan hunter memilih dungeon yang sesuai level, memberikan rewards, dan menghapus dungeon dari shared memory. Jika EXP mencapai 500, hunter naik level. Penanganan kesalahan memastikan pilihan dungeon valid.

h. Karena persaingan antar hunter semakin ketat, Sung Jin Woo mengimplementasikan fitur dimana hunter dapat memilih untuk bertarung dengan hunter lain. Tingkat kekuatan seorang hunter bisa dihitung melalui total stats yang dimiliki hunter tersebut (ATK+HP+DEF). Jika hunter menang, maka hunter tersebut akan mendapatkan semua stats milik lawan dan lawannya akan terhapus dari sistem. Jika kalah, hunter tersebutlah yang akan dihapus dari sistem dan semua statsnya akan diberikan kepada hunter yang dilawannya.

```bash
// hunter.c
void battle(struct Hunter *self, struct SystemData *data) {
    printf(CLEAR_SCREEN);
    print_header(RED "HUNTER BATTLE ARENA" YELLOW);
    int available = 0;
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, self->username) != 0) {
            available++;
            printf(" " CYAN "[%d]" RESET " %-20s | " YELLOW "Lv:" RESET "%2d | " 
                   RED "ATK:" RESET "%3d | " GREEN "HP:" RESET "%3d | " BLUE "DEF:" RESET "%2d        \n",
                i, data->hunters[i].username, data->hunters[i].level, data->hunters[i].atk,
                data->hunters[i].hp, data->hunters[i].def);
        }
    }
    if (!available) {
        printf(" " RED "No other hunters available to battle." RESET "                     \n");
        print_footer();
        return;
    }
    printf(" Choose target (index): ");
    int target;
    scanf("%d", &target);
    getchar();
    if (target < 0 || target >= data->num_hunters || strcmp(data->hunters[target].username, self->username) == 0) {
        printf(" " RED "Invalid target selection. Please try again." RESET "               \n");
        print_footer();
        return;
    }
    int self_power = self->atk + self->hp + self->def;
    int target_power = data->hunters[target].atk + data->hunters[target].hp + data->hunters[target].def;
    if (self_power > target_power) {
        self->atk += data->hunters[target].atk;
        self->hp += data->hunters[target].hp;
        self->def += data->hunters[target].def;
        for (int i = target; i < data->num_hunters - 1; i++) {
            data->hunters[i] = data->hunters[i + 1];
        }
        data->num_hunters--;
        printf(" " GREEN "🏆 VICTORY! " RESET "You defeated %s and gained their powers!      \n", 
               data->hunters[target].username);
    } else if (self_power < target_power) {
        for (int i = 0; i < data->num_hunters; i++) {
            if (strcmp(data->hunters[i].username, data->hunters[target].username) == 0) {
                data->hunters[i].atk += self->atk;
                data->hunters[i].hp += self->hp;
                data->hunters[i].def += self->def;
                break;
            }
        }
        for (int i = 0; i < data->num_hunters; i++) {
            if (strcmp(data->hunters[i].username, self->username) == 0) {
                for (int j = i; j < data->num_hunters - 1; j++) {
                    data->hunters[j] = data->hunters[j + 1];
                }
                data->num_hunters--;
                break;
            }
        }
        printf(" " RED "❌ DEFEAT! " RESET "You lost to %s and your account is deleted!    \n", 
               data->hunters[target].username);
        print_footer();
        exit(0);
    } else {
        printf(" " YELLOW "DRAW! No winner, battle ends in a tie.\n" RESET);
    }
    print_footer();
}
```
Fungsi `battle` memungkinkan hunter memilih lawan dan membandingkan total stats. Pemenang mendapat stats lawan, dan yang kalah dihapus dari shared memory. Jika seri, tidak ada perubahan. Penanganan kesalahan memastikan pilihan lawan valid.

i. Saat sedang memonitoring sistem, Sung Jin Woo melihat beberapa hunter melakukan kecurangan di dalam sistem. Ia menambahkan fitur di sistem yang membuat dia dapat melarang hunter tertentu untuk melakukan raid atau battle. Karena masa percobaan tak bisa berlangsung selamanya 😇, Sung Jin Woo pun juga menambahkan konfigurasi agar fiturnya dapat memperbolehkan hunter itu melakukan raid atau battle lagi.

j. Setelah beberapa pertimbangan, untuk memberikan kesempatan kedua bagi hunter yang ingin bertobat dan memulai dari awal, Sung Jin Woo juga menambahkan fitur di sistem yang membuat dia bisa mengembalikan stats hunter tertentu ke nilai awal. 

```bash
// system.c
void ban_or_unban(struct SystemData *data) {
    printf(CLEAR_SCREEN);
    print_header(RED "BAN/UNBAN HUNTER" YELLOW);
    printf(" " CYAN "Current Hunters:" RESET "                                        \n");
    for (int i = 0; i < data->num_hunters; i++) {
        char status[20];
        sprintf(status, "%s", data->hunters[i].banned ? RED "BANNED" RESET : GREEN "ACTIVE" RESET);
        printf(" " CYAN "[%d]" RESET " %-20s | %-10s                      \n", 
            i + 1, data->hunters[i].username, status);
    }
    printf(" Enter hunter username to toggle ban: ");
    char name[50];
    scanf("%s", name);
    getchar();
    int found = 0;
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, name) == 0) {
            data->hunters[i].banned = !data->hunters[i].banned;
            found = 1;
            printf(" " YELLOW "Hunter %s is now %s" RESET "                              \n", 
                   name, data->hunters[i].banned ? RED "BANNED" RESET : GREEN "UNBANNED" RESET);
            break;
        }
    }
    if (!found) {
        printf(" " RED "Hunter not found. Please check the username." RESET "               \n");
    }
    print_footer();
}
```
Fungsi `ban_or_unban` mengubah status banned hunter, mencegah atau mengizinkan raid/battle. Fungsi `reset_hunter_stats` mengembalikan stats ke awal (Level=1, EXP=0, ATK=10, HP=100, DEF=5, banned=0). Kedua fungsi memperbarui shared memory dan menangani kesalahan untuk username tidak valid.

k. Untuk membuat sistem lebih menarik dan tidak membosankan, Sung Jin Woo menambahkan fitur notifikasi dungeon di setiap hunter. Saat diaktifkan, akan muncul informasi tentang semua dungeon yang terbuka dan akan terus berganti setiap 3 detik.

```bash
// hunter.c
void dungeon_notification(struct Hunter *hunter, struct SystemData *data) {
    signal(SIGINT, stop_notification);
    running = 1;
    int current_index = 0;
    int toggle = 0;
    while (running) {
        check_banned_status(hunter, data);
        printf(CLEAR_SCREEN);
        printf(BLUE);
        printf("╔══════════════════════════════════════════════════════════════════════════════╗\n");
        printf("                " WHITE "                HUNTER TERMINAL" BLUE "                         \n");
        printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
        if (data->num_dungeons > 0) {
            if (toggle % 2 == 0) {
                printf(MAGENTA "  🌟 Dungeon Alert!🌟 " RESET);
            } else {
                printf(YELLOW "  ⚡ Dungeon Alert!⚡ " RESET);
            }
            printf(GREEN "An " CYAN "%s" GREEN " for minimum level " YELLOW "%d" GREEN " open\n" RESET,
                   data->dungeons[current_index].name, data->dungeons[current_index].min_level);
            data->current_notification_index = current_index;
        } else {
            printf(RED "No dungeons available\n" RESET);
        }
        printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
        printf("  " WHITE "1" BLUE " 📊  " WHITE "Show My Stats\n");
        printf("  " WHITE "2" BLUE " 🔍  " WHITE "Show Available Dungeons\n");
        printf("  " WHITE "3" BLUE " ⚔️  " WHITE "Raid Dungeon\n");
        printf("  " WHITE "4" BLUE " 🏆  " WHITE "Battle Another Hunter\n");
        printf("  " WHITE "5" BLUE " 🔔  " WHITE "Start Dungeon Notification\n");
        printf("  " WHITE "6" BLUE " 🚪  " WHITE "Exit\n");
        printf("╚══════════════════════════════════════════════════════════════════════════════╝\n");
        printf(WHITE "Logged in as: " CYAN "%s" WHITE " (Level %d)\n" RESET, hunter->username, hunter->level);
        printf("\nPress Ctrl+C to stop notifications\n");
        if (data->num_dungeons > 0) {
            current_index = (current_index + 1) % data->num_dungeons;
        }
        toggle++;
        fflush(stdout);
        sleep(3);
    }
}
```
Fungsi `dungeon_notification` menampilkan notifikasi dungeon setiap 3 detik, berpindah antar dungeon di shared memory. Warna notifikasi berganti untuk daya tarik visual. Notifikasi berhenti dengan Ctrl+C, dan status banned diperiksa setiap siklus.

l. Terakhir, untuk menambah keamanan sistem agar data hunter tidak bocor, Sung Jin Woo melakukan konfigurasi agar setiap kali sistem dimatikan, maka semua shared memory yang sedang berjalan juga akan ikut terhapus. 

```bash
// system.c
void cleanup_shared_memory() {
    printf("Cleaning up shared memory...\n");
    struct SystemData *data = shmat(shmid, NULL, 0);
    if (data != (void*)-1) {
        shmdt(data);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Failed to delete shared memory");
        printf("shmid: %d, errno: %d\n", shmid, errno);
    } else {
        printf("✓ Shared memory deleted (ID: %d)\n", shmid);
    }
}
```
Fungsi `cleanup_shared_memory` menghapus shared memory dengan `shmctl` saat sistem dimatikan melalui `SIGINT` atau `SIGTERM`. Ini memastikan tidak ada data hunter atau dungeon yang tersisa, meningkatkan keamanan sistem.
