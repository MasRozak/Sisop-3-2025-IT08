# Sisop-3-2025-IT08

Anggota IT08:
| Nama | NRP |
| ---------------------- | ---------- |
| Arya Bisma Putra Refman | 5027241036 |
| Moch. Rizki Nasrullah | 5027241038 |
| Nadia Fauziazahra Kusumastuti | 5027241094 |
----

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

3. Passive: 
Jika senjata yang dipakai memiliki Passive setiap kali passive tersebut menyala, maka tunjukan bahwa passive tersebut aktif.

    - Menu Utama (Error Handling untuk Pilihan Tidak Valid)
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
