#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>

#define SHM_KEY 0x1234
#define MAX_HUNTER 10
#define DUNGEON_KEY 0x4321
#define MAX_DUNGEON 10

#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"

#define CLEAR_SCREEN "\033[2J\033[H"

typedef struct {
    int used;
    char username[32];
    int level;
    int exp;
    int atk;
    int hp;
    int def;
    int online;
    int banned;
} HunterData;

typedef struct {
    int used;
    char name[64];
    int min_level;
    int reward_exp;
    int reward_atk;
    int reward_hp;
    int reward_def;
} Dungeon;

int running = 1;

void print_header(const char *title);
void print_footer();
void stop_notification(int sig);
void show_dungeons(HunterData *hunter, Dungeon *dungeons);
void raid_dungeon(HunterData *hunter, Dungeon *dungeons);
void dungeon_notification(HunterData *hunter, Dungeon *dungeons);
void battle(HunterData *self, HunterData *hunters);
HunterData* login(HunterData *hunters);
HunterData* register_hunter(HunterData *hunters);
void show_hunter_stats(HunterData *hunter);

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

void show_dungeons(HunterData *hunter, Dungeon *dungeons) {
    printf(CLEAR_SCREEN);
    print_header(MAGENTA "AVAILABLE DUNGEONS" YELLOW);
    
    int found = 0;
    for (int i = 0; i < MAX_DUNGEON; i++) {
        if (dungeons[i].used && hunter->level >= dungeons[i].min_level) {
            found++;
            printf(" " CYAN "[%d]" RESET " %-20s | " YELLOW "Min Lv:" RESET "%d | " MAGENTA "EXP:" RESET "%3d | " 
                   RED "ATK:" RESET "%3d | " GREEN "HP:" RESET "%3d | " BLUE "DEF:" RESET "%2d \n",
                i + 1,
                dungeons[i].name,
                dungeons[i].min_level,
                dungeons[i].reward_exp,
                dungeons[i].reward_atk,
                dungeons[i].reward_hp,
                dungeons[i].reward_def
            );
        }
    }
    
    if (!found) {
        printf(" " RED "No available dungeon for your level." RESET "                     \n");
        printf(" " YELLOW "Try to level up to access more dungeons!" RESET "                  \n");
    }
    
    print_footer();
}

void raid_dungeon(HunterData *hunter, Dungeon *dungeons) {
    printf(CLEAR_SCREEN);
    print_header(GREEN "RAID DUNGEON" YELLOW);
    
    int found = 0;
    for (int i = 0; i < MAX_DUNGEON; i++) {
        if (dungeons[i].used && hunter->level >= dungeons[i].min_level) {
            found++;
            printf(" " CYAN "[%d]" RESET " %-20s | " YELLOW "Min Lv:" RESET "%d | " MAGENTA "EXP:" RESET "%3d | " 
                   RED "ATK:" RESET "%3d | " GREEN "HP:" RESET "%3d | " BLUE "DEF:" RESET "%2d \n",
                i + 1,
                dungeons[i].name,
                dungeons[i].min_level,
                dungeons[i].reward_exp,
                dungeons[i].reward_atk,
                dungeons[i].reward_hp,
                dungeons[i].reward_def
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
    
    if (id < 0 || id >= MAX_DUNGEON || !dungeons[id].used || hunter->level < dungeons[id].min_level) {
        printf("                                                                \n");
        printf(" " RED "Invalid dungeon selected. Please try again." RESET "               \n");
        print_footer();
        return;
    }
    
    printf("                                                                \n");
    printf(" " CYAN "Raiding dungeon: %s" RESET "                              \n", dungeons[id].name);
    printf("                                                                \n");
    
    int old_level = hunter->level;
    int old_exp = hunter->exp;
    int old_atk = hunter->atk;
    int old_hp = hunter->hp;
    int old_def = hunter->def;
    
    hunter->exp += dungeons[id].reward_exp;
    hunter->atk += dungeons[id].reward_atk;
    hunter->hp += dungeons[id].reward_hp;
    hunter->def += dungeons[id].reward_def;
    
    bool level_up = false;
    if (hunter->exp >= 500) {
        hunter->level++;
        hunter->exp = 0;
        level_up = true;
    }
    
    dungeons[id].used = 0;
    
    printf(" " GREEN "⚔️  DUNGEON CLEARED SUCCESSFULLY! ⚔️" RESET "                       \n");
    printf("                                                                \n");
    printf(" " YELLOW "REWARDS:" RESET "                                                    \n");
    printf("   🌟 EXP: " CYAN "+%d" RESET " (%d → %d)                                    \n", 
           dungeons[id].reward_exp, old_exp, hunter->exp);
    printf("   ⚔️  ATK: " RED "+%d" RESET " (%d → %d)                                    \n", 
           dungeons[id].reward_atk, old_atk, hunter->atk);
    printf("   ❤️  HP:  " GREEN "+%d" RESET " (%d → %d)                                  \n", 
           dungeons[id].reward_hp, old_hp, hunter->hp);
    printf("   🛡️  DEF: " BLUE "+%d" RESET " (%d → %d)                                   \n", 
           dungeons[id].reward_def, old_def, hunter->def);
    
    if (level_up) {
        printf("                                                                \n");
        printf(" " CYAN "🎉 LEVEL UP! " RESET "You are now " YELLOW "level %d" RESET "!                         \n", 
               hunter->level);
    }
    
    print_footer();
}

void dungeon_notification(HunterData *hunter, Dungeon *dungeons) {
    signal(SIGINT, stop_notification);
    
    printf(CLEAR_SCREEN);
    print_header(CYAN "DUNGEON NOTIFICATION SYSTEM" YELLOW);
    printf(" " GREEN "✓ " RESET "Notification started. Press Ctrl+C to stop.               \n");
    print_footer();
    
    while (running) {
        printf(CLEAR_SCREEN);
        print_header(MAGENTA "LIVE DUNGEON UPDATES" YELLOW);
        
        int found = 0;
        for (int i = 0; i < MAX_DUNGEON; i++) {
            if (dungeons[i].used && hunter->level >= dungeons[i].min_level) {
                found++;
                printf(" " CYAN "[%d]" RESET " %-20s | " YELLOW "Min Lv:" RESET "%d | " MAGENTA "EXP:" RESET "%3d | " 
                       RED "ATK:" RESET "%3d | " GREEN "HP:" RESET "%3d | " BLUE "DEF:" RESET "%2d \n",
                    i + 1,
                    dungeons[i].name,
                    dungeons[i].min_level,
                    dungeons[i].reward_exp,
                    dungeons[i].reward_atk,
                    dungeons[i].reward_hp,
                    dungeons[i].reward_def
                );
            }
        }
        
        if (!found) {
            printf(" " RED "No available dungeon for your level." RESET "                     \n");
            printf(" " YELLOW "Try to level up to access more dungeons!" RESET "                  \n");
        }
        
        printf("                                                                \n");
        printf(" " BLUE "Refreshing in 3 seconds..." RESET " (Press Ctrl+C to stop)          \n");
        printf(YELLOW "╚════════════════════════════════════════════════════════════════╝\n" RESET);
        
        sleep(3);
    }
    
    printf(CLEAR_SCREEN);
    print_header(RED "NOTIFICATION STOPPED" YELLOW);
    printf(" Dungeon notification service has been stopped.                \n");
    print_footer();
}

void battle(HunterData *self, HunterData *hunters) {
    printf(CLEAR_SCREEN);
    print_header(RED "HUNTER BATTLE ARENA" YELLOW);
    
    int available = 0;
    for (int i = 0; i < MAX_HUNTER; i++) {
        if (hunters[i].used && hunters[i].online && strcmp(hunters[i].username, self->username) != 0) {
            available++;
            printf(" " CYAN "[%d]" RESET " %-20s | " YELLOW "Lv:" RESET "%2d | " 
                   RED "ATK:" RESET "%3d | " GREEN "HP:" RESET "%3d | " BLUE "DEF:" RESET "%2d        \n",
                i,
                hunters[i].username,
                hunters[i].level,
                hunters[i].atk,
                hunters[i].hp,
                hunters[i].def
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
    
    if (target < 0 || target >= MAX_HUNTER || !hunters[target].used || !hunters[target].online || 
        strcmp(hunters[target].username, self->username) == 0) {
        printf("                                                                \n");
        printf(" " RED "Invalid target selection. Please try again." RESET "               \n");
        print_footer();
        return;
    }
    
    printf("                                                                \n");
    printf(" " CYAN "⚔️  BATTLE INITIATION ⚔️" RESET "                                    \n");
    printf("                                                                \n");
    printf(" " GREEN "%s" RESET " vs " RED "%s" RESET "                                    \n", 
           self->username, hunters[target].username);
    printf("                                                                \n");
    
    int self_power = self->atk + self->hp + self->def;
    int target_power = hunters[target].atk + hunters[target].hp + hunters[target].def;
    
    printf(" " GREEN "Your Power:" RESET " %d                                             \n", self_power);
    printf(" " RED "Enemy Power:" RESET " %d                                            \n", target_power);
    printf("                                                                \n");
    
    if (self_power >= target_power) {
        int old_atk = self->atk;
        int old_hp = self->hp;
        int old_def = self->def;
        
        self->atk += hunters[target].atk / 2;  
        self->hp += hunters[target].hp / 2;
        self->def += hunters[target].def / 2;
        hunters[target].used = 0;  
        
        printf(" " GREEN "🏆 VICTORY! " RESET "You defeated %s and gained their powers!      \n", 
               hunters[target].username);
        printf("                                                                \n");
        printf(" " YELLOW "BATTLE REWARDS:" RESET "                                            \n");
        printf("   ⚔️  ATK: " RED "+%d" RESET " (%d → %d)                                    \n", 
               self->atk - old_atk, old_atk, self->atk);
        printf("   ❤️  HP:  " GREEN "+%d" RESET " (%d → %d)                                  \n", 
               self->hp - old_hp, old_hp, self->hp);
        printf("   🛡️  DEF: " BLUE "+%d" RESET " (%d → %d)                                   \n", 
               self->def - old_def, old_def, self->def);
    } else {
        hunters[target].atk += self->atk / 2; 
        hunters[target].hp += self->hp / 2;
        hunters[target].def += self->def / 2;
        self->used = 0;
        
        printf(" " RED "❌ DEFEAT! " RESET "You lost to %s and your account is deleted!    \n", 
               hunters[target].username);
        printf("                                                                \n");
        printf(" " YELLOW "You will need to create a new hunter account." RESET "              \n");
    }
    
    print_footer();
    
    if (self_power < target_power) {
        printf(CLEAR_SCREEN);
        print_header(RED "HUNTER DEFEATED" YELLOW);
        printf(" Your hunter has been eliminated from the game.              \n");
        printf(" Please restart the application to create a new hunter.      \n");
        print_footer();
        exit(0);
    }
}

void show_hunter_stats(HunterData *hunter) {
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

HunterData* login(HunterData *hunters) {
    char uname[32];
    printf(" Masukkan username: ");
    scanf("%s", uname);
    getchar();

    for (int i = 0; i < MAX_HUNTER; i++) {
        if (hunters[i].used && strcmp(hunters[i].username, uname) == 0) {
            if (hunters[i].banned) {
                printf(RED "⛔ Akun diblokir. Hubungi admin.\n" RESET);
                sleep(2);
                return NULL;
            }
            hunters[i].online = 1;
            printf(GREEN "✓ Selamat datang kembali, %s!\n" RESET, uname);
            sleep(2);
            return &hunters[i];
        }
    }

    printf(RED "❌ Username tidak ditemukan.\n" RESET);
    sleep(2);
    return NULL;
}

HunterData* register_hunter(HunterData *hunters) {
    char uname[32];
    printf(" Buat username baru: ");
    scanf("%s", uname);
    getchar();

    for (int i = 0; i < MAX_HUNTER; i++) {
        if (hunters[i].used && strcmp(hunters[i].username, uname) == 0) {
            printf(RED "❌ Username sudah digunakan.\n" RESET);
            sleep(2);
            return NULL;
        }
    }

    for (int i = 0; i < MAX_HUNTER; i++) {
        if (!hunters[i].used) {
            hunters[i].used = 1;
            strcpy(hunters[i].username, uname);
            hunters[i].level = 1;
            hunters[i].exp = 0;
            hunters[i].atk = 10;
            hunters[i].hp = 100;
            hunters[i].def = 5;
            hunters[i].online = 1;
            hunters[i].banned = 0;

            printf(GREEN "✓ Akun berhasil dibuat! Selamat datang, %s!\n" RESET, uname);
            sleep(2);
            return &hunters[i];
        }
    }

    printf(RED "❌ Server penuh. Tidak bisa mendaftar lagi.\n" RESET);
    sleep(2);
    return NULL;
}

int menu_awal() {
    int pilihan;
    printf(CLEAR_SCREEN);
    printf(YELLOW);
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("                   " CYAN "HUNTER ASSOCIATION" YELLOW " MENU                    \n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf(" " CYAN "[1]" RESET " Register                                             \n");
    printf(" " CYAN "[2]" RESET " Login                                                \n");
    printf(" " CYAN "[3]" RESET " Exit                                                 \n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf(" Pilih opsi (1-3): ");
    scanf("%d", &pilihan);
    getchar(); 
    return pilihan;
}

int main() {
    printf(CLEAR_SCREEN);
    printf(YELLOW);
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("                    " CYAN "HUNTER ASSOCIATION" YELLOW "                       \n");
    printf("                 " MAGENTA "Created by Sung Jin Woo" YELLOW "                    \n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf(RESET);

    int shmid = shmget(SHM_KEY, sizeof(HunterData) * MAX_HUNTER, 0666);
    if (shmid < 0) {
        shmid = shmget(SHM_KEY, sizeof(HunterData) * MAX_HUNTER, IPC_CREAT | 0666);
        if (shmid < 0) {
            perror(RED "ERROR: Cannot create shared memory for hunters\n" RESET);
            exit(1);
        }
        printf(GREEN "✓ " RESET "Created new shared memory for hunters\n");
    } else {
        printf(GREEN "✓ " RESET "Connected to existing hunter shared memory\n");
    }

    HunterData *hunters = shmat(shmid, NULL, 0);
    if (hunters == (void *) -1) {
        perror(RED "ERROR: Cannot attach hunter memory\n" RESET);
        exit(1);
    }

    int dshmid = shmget(DUNGEON_KEY, sizeof(Dungeon) * MAX_DUNGEON, 0666);
    if (dshmid < 0) {
        dshmid = shmget(DUNGEON_KEY, sizeof(Dungeon) * MAX_DUNGEON, IPC_CREAT | 0666);
        if (dshmid < 0) {
            perror(RED "ERROR: Cannot create shared memory for dungeons\n" RESET);
            exit(1);
        }
        printf(GREEN "✓ " RESET "Created new shared memory for dungeons\n");
    } else {
        printf(GREEN "✓ " RESET "Connected to existing dungeon shared memory\n");
    }

    Dungeon *dungeons = shmat(dshmid, NULL, 0);
    if (dungeons == (void *) -1) {
        perror(RED "ERROR: Cannot attach dungeon memory\n" RESET);
        exit(1);
    }

    printf(GREEN "\n⚔️  System is ready! Connecting to server..." RESET);
    sleep(2);

    HunterData *me = NULL;
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
            me = register_hunter(hunters);
        } else if (opt == 2) {
            me = login(hunters);
        } else if (opt == 3) {
            printf(YELLOW "Goodbye, Hunter!\n" RESET);
            shmdt(hunters);
            shmdt(dungeons);
            exit(0);
        } else {
            printf(RED "Invalid choice. Try again.\n" RESET);
            sleep(1);
        }
    }

    while (1) {
        printf(CLEAR_SCREEN);
        printf(BLUE);
        printf("╔════════════════════════════════════════════════════════════════╗\n");
        printf("                    " WHITE "HUNTER TERMINAL" BLUE "                         \n");
        printf("╠════════════════════════════════════════════════════════════════╣\n");
        printf("  " WHITE "1" BLUE " 📊  " WHITE "Show My Stats\n");
        printf("  " WHITE "2" BLUE " 🔍  " WHITE "Show Available Dungeons\n");
        printf("  " WHITE "3" BLUE " ⚔️  " WHITE "Raid Dungeon\n");
        printf("  " WHITE "4" BLUE " 🏆  " WHITE "Battle Another Hunter\n");
        printf("  " WHITE "5" BLUE " 🔔  " WHITE "Start Dungeon Notification\n");
        printf("  " WHITE "6" BLUE " 🚪  " WHITE "Exit\n");
        printf("╚════════════════════════════════════════════════════════════════╝\n");
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
            case 2: show_dungeons(me, dungeons); break;
            case 3: raid_dungeon(me, dungeons); break;
            case 4: battle(me, hunters); break;
            case 5: dungeon_notification(me, dungeons); break;
            case 6:
                printf(CLEAR_SCREEN);
                print_header(YELLOW "LOGGING OUT");
                me->online = 0;
                printf(" Goodbye, hunter! Your progress has been saved.\n");
                print_footer();
                shmdt(hunters);
                shmdt(dungeons);
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
