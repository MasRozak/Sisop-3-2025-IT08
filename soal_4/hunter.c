#include "shm_common.h"
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"

#define CLEAR_SCREEN "\033[2J\033[H"

int running = 1;

void print_header(const char *title);
void print_footer();
void stop_notification(int sig);
void show_dungeons(struct Hunter *hunter, struct SystemData *data);
void raid_dungeon(struct Hunter *hunter, struct SystemData *data);
void dungeon_notification(struct Hunter *hunter, struct SystemData *data);
void battle(struct Hunter *self, struct SystemData *data);
struct Hunter* login(struct SystemData *data);
struct Hunter* register_hunter(struct SystemData *data);
void show_hunter_stats(struct Hunter *hunter);

void print_header(const char *title) {
    printf(YELLOW);
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf(" %s", title);
    for(int i = strlen(title); i < 60; i++) printf(" ");
    printf("\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf(RESET);
}

void print_footer() {
    printf(YELLOW);
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf(RESET);
    printf("\nPress Enter to continue...");
    getchar();
}

void stop_notification(int sig) {
    running = 0;
}

void show_dungeons(struct Hunter *hunter, struct SystemData *data) {
    printf(CLEAR_SCREEN);
    print_header(MAGENTA "AVAILABLE DUNGEONS" YELLOW);
    
    int found = 0;
    for (int i = 0; i < data->num_dungeons; i++) {
        if (hunter->level >= data->dungeons[i].min_level) {
            found++;
            printf(" " CYAN "[%d]" RESET " %-20s | " YELLOW "Min Lv:" RESET "%d | " MAGENTA "EXP:" RESET "%3d | " 
                   RED "ATK:" RESET "%3d | " GREEN "HP:" RESET "%3d | " BLUE "DEF:" RESET "%2d \n",
                i + 1,
                data->dungeons[i].name,
                data->dungeons[i].min_level,
                data->dungeons[i].exp,
                data->dungeons[i].atk,
                data->dungeons[i].hp,
                data->dungeons[i].def
            );
        }
    }
    
    if (!found) {
        printf(" " RED "No available dungeon for your level." RESET "                     \n");
        printf(" " YELLOW "Try to level up to access more dungeons!" RESET "                  \n");
    }
    
    print_footer();
}

void raid_dungeon(struct Hunter *hunter, struct SystemData *data) {
    printf(CLEAR_SCREEN);
    print_header(GREEN "RAID DUNGEON" YELLOW);
    
    int found = 0;
    for (int i = 0; i < data->num_dungeons; i++) {
        if (hunter->level >= data->dungeons[i].min_level) {
            found++;
            printf(" " CYAN "[%d]" RESET " %-20s | " YELLOW "Min Lv:" RESET "%d | " MAGENTA "EXP:" RESET "%3d | " 
                   RED "ATK:" RESET "%3d | " GREEN "HP:" RESET "%3d | " BLUE "DEF:" RESET "%2d \n",
                i + 1,
                data->dungeons[i].name,
                data->dungeons[i].min_level,
                data->dungeons[i].exp,
                data->dungeons[i].atk,
                data->dungeons[i].hp,
                data->dungeons[i].def
            );
        }
    }
    
    if (!found) {
        printf(" " RED "No available dungeon for your level." RESET "                     \n");
        printf(" " YELLOW "Try to level up to access more dungeons!" RESET "                  \n");
        print_footer();
        return;
    }
    
    printf("                                                                \n");
    printf(" Choose dungeon to raid (number): ");
    int id; 
    scanf("%d", &id);
    getchar();
    id--;
    
    if (id < 0 || id >= data->num_dungeons || hunter->level < data->dungeons[id].min_level) {
        printf("                                                                \n");
        printf(" " RED "Invalid dungeon selected. Please try again." RESET "               \n");
        print_footer();
        return;
    }
    
    printf("                                                                \n");
    printf(" " CYAN "Raiding dungeon: %s" RESET "                              \n", data->dungeons[id].name);
    printf("                                                                \n");
    
    int old_level = hunter->level;
    int old_exp = hunter->exp;
    int old_atk = hunter->atk;
    int old_hp = hunter->hp;
    int old_def = hunter->def;
    
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
    printf("                                                                \n");
    printf(" " YELLOW "REWARDS:" RESET "                                                    \n");
    printf("   🌟 EXP: " CYAN "+%d" RESET " (%d → %d)                                    \n", 
           data->dungeons[id].exp, old_exp, hunter->exp);
    printf("   ⚔️  ATK: " RED "+%d" RESET " (%d → %d)                                    \n", 
           data->dungeons[id].atk, old_atk, hunter->atk);
    printf("   ❤️  HP:  " GREEN "+%d" RESET " (%d → %d)                                  \n", 
           data->dungeons[id].hp, old_hp, hunter->hp);
    printf("   🛡️  DEF: " BLUE "+%d" RESET " (%d → %d)                                   \n", 
           data->dungeons[id].def, old_def, hunter->def);
    
    if (level_up) {
        printf("                                                                \n");
        printf(" " CYAN "🎉 LEVEL UP! " RESET "You are now " YELLOW "level %d" RESET "!                         \n", 
               hunter->level);
    }
    
    print_footer();
}

void dungeon_notification(struct Hunter *hunter, struct SystemData *data) {
    signal(SIGINT, stop_notification);
    
    running = 1;
    int current_index = 0;
    int toggle = 0;
    
    while (running) {
        printf(CLEAR_SCREEN); 
        printf(BLUE);
        printf("╔══════════════════════════════════════════════════════════════════════════════╗\n");
        printf("                " WHITE "                HUNTER TERMINAL" BLUE "                         \n");
        printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
        
        if (data->num_dungeons > 0) {
            if (toggle % 2 == 0) {
                printf(MAGENTA "  🌟 Dungeon Alert!🌟 " RESET);
            } else {
                printf(YELLOW "  ⚡ Dungeon Allert!⚡ " RESET);
            }
            
            printf(GREEN "An " CYAN "%s" GREEN " for minimum level " YELLOW "%d" GREEN " open\n" RESET,
                   data->dungeons[current_index].name,
                   data->dungeons[current_index].min_level);
                   
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

void battle(struct Hunter *self, struct SystemData *data) {
    printf(CLEAR_SCREEN);
    print_header(RED "HUNTER BATTLE ARENA" YELLOW);
    
    int available = 0;
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, self->username) != 0) {
            available++;
            printf(" " CYAN "[%d]" RESET " %-20s | " YELLOW "Lv:" RESET "%2d | " 
                   RED "ATK:" RESET "%3d | " GREEN "HP:" RESET "%3d | " BLUE "DEF:" RESET "%2d        \n",
                i,
                data->hunters[i].username,
                data->hunters[i].level,
                data->hunters[i].atk,
                data->hunters[i].hp,
                data->hunters[i].def
            );
        }
    }
    
    if (!available) {
        printf(" " RED "No other hunters available to battle." RESET "                     \n");
        print_footer();
        return;
    }
    
    printf("                                                                \n");
    printf(" Choose target (index): ");
    int target; 
    scanf("%d", &target);
    getchar();
    
    if (target < 0 || target >= data->num_hunters || strcmp(data->hunters[target].username, self->username) == 0) {
        printf("                                                                \n");
        printf(" " RED "Invalid target selection. Please try again." RESET "               \n");
        print_footer();
        return;
    }
    
    printf("                                                                \n");
    printf(" " CYAN "⚔️  BATTLE INITIATION ⚔️" RESET "                                    \n");
    printf("                                                                \n");
    printf(" " GREEN "%s" RESET " vs " RED "%s" RESET "                                    \n", 
           self->username, data->hunters[target].username);
    printf("                                                                \n");
    
    int self_power = self->atk + self->hp + self->def;
    int target_power = data->hunters[target].atk + data->hunters[target].hp + data->hunters[target].def;
    
    printf(" " GREEN "Your Power:" RESET " %d                                             \n", self_power);
    printf(" " RED "Enemy Power:" RESET " %d                                            \n", target_power);
    printf("                                                                \n");
    
    if (self_power > target_power) {
        int old_atk = self->atk;
        int old_hp = self->hp;
        int old_def = self->def;
        
        char target_name[50];
        strcpy(target_name, data->hunters[target].username);
        int target_atk = data->hunters[target].atk;
        int target_hp = data->hunters[target].hp;
        int target_def = data->hunters[target].def;
        
        self->atk += target_atk;  
        self->hp += target_hp;
        self->def += target_def;
        
        for (int i = target; i < data->num_hunters - 1; i++) {
            data->hunters[i] = data->hunters[i + 1];
        }
        data->num_hunters--;
        
        printf(" " GREEN "🏆 VICTORY! " RESET "You defeated %s and gained their powers!      \n", 
               target_name);
        printf("                                                                \n");
        printf(" " YELLOW "BATTLE REWARDS:" RESET "                                            \n");
        printf("   ⚔️  ATK: " RED "+%d" RESET " (%d → %d)                                    \n", 
               target_atk, old_atk, self->atk);
        printf("   ❤️  HP:  " GREEN "+%d" RESET " (%d → %d)                                  \n", 
               target_hp, old_hp, self->hp);
        printf("   🛡️  DEF: " BLUE "+%d" RESET " (%d → %d)                                   \n", 
               target_def, old_def, self->def);
    } else if (self_power < target_power) {
        int self_atk = self->atk;
        int self_hp = self->hp;
        int self_def = self->def;
        
        for (int i = 0; i < data->num_hunters; i++) {
            if (strcmp(data->hunters[i].username, data->hunters[target].username) == 0) {
                data->hunters[i].atk += self_atk;
                data->hunters[i].hp += self_hp;
                data->hunters[i].def += self_def;
                break;
            }
        }

        char target_name[50];
        strcpy(target_name, data->hunters[target].username);

        for (int i = 0; i < data->num_hunters; i++) {
            if (strcmp(data->hunters[i].username, self->username) == 0) {
                for (int j = i; j < data->num_hunters - 1; j++) {
                    data->hunters[j] = data->hunters[j + 1];
                }
                data->num_hunters--;
                break;
            }
        }

        printf(" " RED "❌ DEFEAT! " RESET "You lost to %s and your account is deleted!    \n", target_name);
        printf("                                                                \n");
        printf(" " YELLOW "BATTLE REWARDS FOR %s:" RESET "                                    \n", target_name);
        printf("   ⚔️  ATK: " RED "+%d" RESET " (%d → %d)                                    \n", 
               self_atk, data->hunters[target].atk - self_atk, data->hunters[target].atk);
        printf("   ❤️  HP:  " GREEN "+%d" RESET " (%d → %d)                                  \n", 
               self_hp, data->hunters[target].hp - self_hp, data->hunters[target].hp);
        printf("   🛡️  DEF: " BLUE "+%d" RESET " (%d → %d)                                   \n", 
               self_def, data->hunters[target].def - self_def, data->hunters[target].def);
        printf(" " YELLOW "You will need to create a new hunter account." RESET "              \n");
        
        print_footer();
        exit(0); 
    } else {
        printf(" " YELLOW "DRAW! No winner, battle ends in a tie.\n" RESET);
    }
    
    print_footer(); 
}

void show_hunter_stats(struct Hunter *hunter) {
    printf(CLEAR_SCREEN);
    print_header(CYAN "HUNTER PROFILE" YELLOW);
    
    printf(" " YELLOW "USERNAME: " RESET "%-20s                               \n", hunter->username);
    printf("                                                                \n");
    printf(" " YELLOW "LEVEL:   " GREEN "%2d" RESET "                                                \n", hunter->level);
    printf(" " YELLOW "EXP:     " CYAN "%3d" RESET "/500                                          \n", hunter->exp);
    printf("                                                                \n");
    printf(" " YELLOW "STATISTICS:" RESET "                                                 \n");
    printf("   ⚔️  ATK: " RED "%3d" RESET "                                               \n", hunter->atk);
    printf("   ❤️  HP:  " GREEN "%3d" RESET "                                               \n", hunter->hp);
    printf("   🛡️  DEF: " BLUE "%2d" RESET "                                                \n", hunter->def);
    printf("                                                                \n");
    printf(" " YELLOW "STATUS:  " RESET "%s                                        \n", 
           hunter->banned ? RED "BANNED" RESET : GREEN "ACTIVE" RESET);
    
    print_footer();
}

struct Hunter* login(struct SystemData *data) {
    char uname[50];
    printf(" Masukkan username: ");
    scanf("%s", uname);
    getchar();

    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, uname) == 0) {
            if (data->hunters[i].banned) {
                printf(RED "⛔ Akun diblokir. Hubungi admin.\n" RESET);
                sleep(2);
                return NULL;
            }
            printf(GREEN "✓ Selamat datang kembali, %s!\n" RESET, uname);
            sleep(2);
            return &data->hunters[i];
        }
    }

    printf(RED "❌ Username tidak ditemukan.\n" RESET);
    sleep(2);
    return NULL;
}

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

int main() {
    printf(CLEAR_SCREEN);
    printf(YELLOW);
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("                    " CYAN "HUNTER ASSOCIATION" YELLOW "                       \n");
    printf("                 " MAGENTA "Created by Sung Jin Woo" YELLOW "                    \n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf(RESET);

    key_t key = get_system_key();
    int shmid = shmget(key, sizeof(struct SystemData), 0666);
    if (shmid < 0) {
        shmid = shmget(key, sizeof(struct SystemData), IPC_CREAT | 0666);
        if (shmid < 0) {
            perror(RED "ERROR: Cannot create shared memory\n" RESET);
            exit(1);
        }
        printf(GREEN "✓ " RESET "Created new shared memory\n");
    } else {
        printf(GREEN "✓ " RESET "Connected to existing shared memory\n");
    }

    struct SystemData *data = shmat(shmid, NULL, 0);
    if (data == (void *) -1) {
        perror(RED "ERROR: Cannot attach memory\n" RESET);
        exit(1);
    }

    printf(GREEN "\n⚔️  System is ready! Connecting to server..." RESET);
    sleep(2);

    struct Hunter *me = NULL;
    while (!me) {
        printf(CLEAR_SCREEN);
        printf(BLUE);
        printf("╔══════════════════════════════════════════════════════════════╗\n");
        printf("                     " WHITE "HUNTER TERMINAL LOGIN" BLUE "                    \n");
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        printf("  " WHITE "1" BLUE " 📝  Register a new Hunter                              \n");
        printf("  " WHITE "2" BLUE " 🔐  Login to your Hunter Account                       \n");
        printf("  " WHITE "3" BLUE " ❌  Exit                                               \n");
        printf("╚══════════════════════════════════════════════════════════════╝\n");
        printf(WHITE "Choose option: " RESET);
        int opt;
        scanf("%d", &opt);
        getchar(); 
        if (opt == 1) {
            me = register_hunter(data);
        } else if (opt == 2) {
            me = login(data);
        } else if (opt == 3) {
            printf(YELLOW "Goodbye, Hunter!\n" RESET);
            shmdt(data);
            exit(0);
        } else {
            printf(RED "Invalid choice. Try again.\n" RESET);
            sleep(1);
        }
    }

    while (1) {
        printf(CLEAR_SCREEN);
        printf(BLUE);
        printf("╔══════════════════════════════════════════════════════════════════════════════╗\n");
        printf("                " WHITE "                HUNTER TERMINAL" BLUE "                         \n");
        printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");

        if (data->num_dungeons > 0) {
            int display_index = data->current_notification_index % data->num_dungeons;
            printf(GREEN "    🌟 Dungeon Alert!🌟 An " CYAN "%s" GREEN " for minimum level " YELLOW "%d" GREEN " open\n" RESET,
                   data->dungeons[display_index].name,
                   data->dungeons[display_index].min_level);
            
            data->current_notification_index = (data->current_notification_index + 1) % data->num_dungeons;
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
        printf(WHITE "Logged in as: " CYAN "%s" WHITE " (Level %d)\n" RESET, me->username, me->level);
        printf("Enter your choice: ");

        int choice;
        scanf("%d", &choice);
        getchar();

        if (me->banned) {
            printf(CLEAR_SCREEN);
            print_header(RED "ACCOUNT BANNED" YELLOW);
            printf(" Your account has been banned. Contact support.           \n");
            print_footer();
            continue;
        }

        switch (choice) {
            case 1: show_hunter_stats(me); break;
            case 2: show_dungeons(me, data); break;
            case 3: raid_dungeon(me, data); break;
            case 4: battle(me, data); break;
            case 5: 
                printf("\n" GREEN "✓ Notification started. Press Ctrl+C to stop.\n" RESET);
                dungeon_notification(me, data);
                break;
            case 6:
                printf(CLEAR_SCREEN);
                print_header(YELLOW "LOGGING OUT");
                printf(" Goodbye, hunter! Your progress has been saved.\n");
                print_footer();
                shmdt(data);
                printf(CLEAR_SCREEN);
                printf(YELLOW);
                printf("╔════════════════════════════════════════════════════════════════╗\n");
                printf("                " RED "CONNECTION TERMINATED" YELLOW "                        \n");
                printf("╚════════════════════════════════════════════════════════════════╝\n");
                printf(RESET);
                exit(0);
            default:
                printf(RED "Invalid choice. Please try again.\n" RESET);
                sleep(1);
        }
    }

    return 0;
}
