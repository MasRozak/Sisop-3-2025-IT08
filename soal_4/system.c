#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>    
#include <unistd.h>  
#include <signal.h>
#include <errno.h>      
#include <sys/types.h>  
#include <sys/ipc.h>   

#define SHM_KEY 0x1234
#define DUNGEON_KEY 0x4321
#define MAX_HUNTER 10
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

int shmid;
int dshmid;
HunterData *hunters;
Dungeon *dungeons;

void cleanup_shared_memory() {
    printf("Cleaning up shared memory...\n");
    
    if (hunters != (void*)-1 && shmdt(hunters) == -1) {
        perror("Failed to detach hunters memory");
    }
    
    if (dungeons != (void*)-1 && shmdt(dungeons) == -1) {
        perror("Failed to detach dungeons memory");
    }
    
    if (shmid != -1) {
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            perror("Failed to delete hunter shared memory");
            printf("Hunter shmid: %d, errno: %d\n", shmid, errno);
        } else {
            printf("✓ Hunter shared memory deleted (ID: %d)\n", shmid);
        }
    }
    
    if (dshmid != -1) {
        if (shmctl(dshmid, IPC_RMID, NULL) == -1) {
            perror("Failed to delete dungeon shared memory");
            printf("Dungeon dshmid: %d, errno: %d\n", dshmid, errno);
        } else {
            printf("✓ Dungeon shared memory deleted (ID: %d)\n", dshmid);
        }
    }
}

void handle_signal(int sig) {
    printf("\nReceived signal %d. Cleaning up shared memory...\n", sig);
    cleanup_shared_memory();
    exit(0);
}

void show_all_hunters(HunterData *hunters);
void ban_or_unban(HunterData *hunters);
void reset_hunter_stats(HunterData *hunters);
void generate_dungeon(Dungeon *dungeons);
void show_all_dungeons(Dungeon *dungeons);
void print_header(const char *title);
void print_footer();
void initialize_memory(HunterData *hunters, Dungeon *dungeons);

int main() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal); 

    shmid = shmget(SHM_KEY, sizeof(HunterData) * MAX_HUNTER, IPC_CREAT | 0666);
    dshmid = shmget(DUNGEON_KEY, sizeof(Dungeon) * MAX_DUNGEON, IPC_CREAT | 0666);

    srand(time(NULL));

    printf(CLEAR_SCREEN);
    printf(YELLOW);
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("                   " CYAN "HUNTER MANAGEMENT SYSTEM" YELLOW "                   \n");
    printf("                " MAGENTA "Created by Sung Jin Woo" YELLOW "                    \n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf(RESET);
    
    shmid = shmget(SHM_KEY, sizeof(HunterData) * MAX_HUNTER, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid == -1) {
        if (errno == EEXIST) {
            shmid = shmget(SHM_KEY, 0, 0);
            printf(YELLOW "Using existing hunter shared memory (ID: %d)\n" RESET, shmid);
        } else {
            perror("shmget hunter failed");
            exit(1);
        }
    }
    printf(GREEN "✓ " RESET "Shared memory Hunter created with ID: %d\n", shmid);

    HunterData *hunters = shmat(shmid, NULL, 0);
    if (hunters == (void *) -1) { 
        printf(RED "ERROR: Could not attach to hunter memory\n" RESET);
        perror("shmat hunter"); 
        exit(1); 
    }

    int dshmid = shmget(DUNGEON_KEY, sizeof(Dungeon) * MAX_DUNGEON, IPC_CREAT | 0666);
    if (dshmid < 0) { 
        printf(RED "ERROR: Could not create shared memory for dungeons\n" RESET);
        perror("shmget dungeon"); 
        exit(1); 
    }
    printf(GREEN "✓ " RESET "Shared memory Dungeon created with ID: %d\n", dshmid);

    Dungeon *dungeons = shmat(dshmid, NULL, 0);
    if (dungeons == (void *) -1) { 
        printf(RED "ERROR: Could not attach to dungeon memory\n" RESET);
        perror("shmat dungeon"); 
        exit(1); 
    }
    
    initialize_memory(hunters, dungeons);
    
    printf(GREEN "\n⚔️  System is ready! ⚔️\n" RESET);
    sleep(2);

    int choice;
    while (1) {
        printf(CLEAR_SCREEN);
        printf(BLUE);
        printf("╔════════════════════════════════════════════════════════════════╗\n");
        printf("                    " WHITE "HUNTER ADMIN SYSTEM" BLUE "                      \n");
        printf("╠════════════════════════════════════════════════════════════════╣\n");
        printf("  " WHITE "1" BLUE " ⚔️  " WHITE "Show All Hunters" BLUE "                                   \n");
        printf("  " WHITE "2" BLUE " 🏰  " WHITE "Show All Dungeons" BLUE "                                  \n");
        printf("  " WHITE "3" BLUE " 🌟  " WHITE "Generate Dungeon" BLUE "                                   \n");
        printf("  " WHITE "4" BLUE " 🚫  " WHITE "Ban/Unban Hunter" BLUE "                                   \n");
        printf("  " WHITE "5" BLUE " 🔄  " WHITE "Reset Hunter Stats" BLUE "                                 \n");
        printf("  " WHITE "6" BLUE " 🚪  " WHITE "Exit" BLUE "                                               \n");
        printf("╚════════════════════════════════════════════════════════════════╝\n");
        printf(WHITE "Enter your choice: " RESET);
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1: show_all_hunters(hunters); break;
            case 2: show_all_dungeons(dungeons); break;
            case 3: generate_dungeon(dungeons); break;
            case 4: ban_or_unban(hunters); break;
            case 5: reset_hunter_stats(hunters); break;
            case 6:
                printf(CLEAR_SCREEN);
                printf(YELLOW);
                printf("╔════════════════════════════════════════════════════════════════╗\n");
                printf("                    " RED "SHUTTING DOWN SYSTEM" YELLOW "                    \n");
                printf("╚════════════════════════════════════════════════════════════════╝\n");
                printf(RESET);
                printf("Detaching from shared memory resources...\n");
                
                printf("Detaching from shared memory resources...\n");
                shmdt(hunters);
                shmdt(dungeons);
                cleanup_shared_memory();

                printf(GREEN "System shutdown complete. Shared memory deleted. Goodbye!\n" RESET);
                printf("Trying to delete shmid: %d\n", shmid);
                printf("Trying to delete dshmid: %d\n", dshmid);
                exit(0);
            default:
                printf(RED "Invalid choice! Try again.\n" RESET);
                sleep(1);
        }
    }
}

void initialize_memory(HunterData *hunters, Dungeon *dungeons) {
    int needs_init = 1;
    
    for (int i = 0; i < MAX_HUNTER; i++) {
        if (hunters[i].used) {
            needs_init = 0;
            break;
        }
    }
    
    if (needs_init) {
        printf(YELLOW "Initializing shared memory for first use...\n" RESET);
        
        for (int i = 0; i < MAX_HUNTER; i++) {
            hunters[i].used = 0;
            hunters[i].online = 0;
            hunters[i].banned = 0;
        }
        
        for (int i = 0; i < MAX_DUNGEON; i++) {
            dungeons[i].used = 0;
        }
        
        printf(GREEN "Memory initialization complete!\n" RESET);
    } else {
        printf(GREEN "Using existing shared memory data.\n" RESET);
    }
}

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

void show_all_hunters(HunterData *hunters) {
    printf(CLEAR_SCREEN);
    print_header(CYAN "REGISTERED HUNTERS LIST" YELLOW);
    
    int count = 0;
    for (int i = 0; i < MAX_HUNTER; i++) {
        if (hunters[i].used) {
            count++;
            char status[20];
            sprintf(status, "%s", hunters[i].banned ? RED "BANNED" RESET : GREEN "ACTIVE" RESET);
            char online[20];
            sprintf(online, "%s", hunters[i].online ? GREEN "ONLINE" RESET : YELLOW "OFFLINE" RESET);
            
            printf(" " CYAN "[%d]" RESET " %-20s | " YELLOW "Lv:" RESET "%2d | " YELLOW "EXP:" RESET "%3d | " 
                   RED "ATK:" RESET "%3d | " GREEN "HP:" RESET "%3d | " BLUE "DEF:" RESET "%2d | %-10s | %-10s \n", 
                i + 1,
                hunters[i].username,
                hunters[i].level,
                hunters[i].exp,
                hunters[i].atk,
                hunters[i].hp,
                hunters[i].def,
                status,
                online
            );
        }
    }
    
    if (count == 0) {
        printf(" " RED "No hunters registered yet..." RESET "                             \n");
    }
    
    print_footer();
}

void ban_or_unban(HunterData *hunters) {
    printf(CLEAR_SCREEN);
    print_header(RED "BAN/UNBAN HUNTER" YELLOW);

    printf(" " CYAN "Current Hunters:" RESET "                                        \n");
    for (int i = 0; i < MAX_HUNTER; i++) {
        if (hunters[i].used) {
            char status[20];
            sprintf(status, "%s", hunters[i].banned ? RED "BANNED" RESET : GREEN "ACTIVE" RESET);
            printf(" " CYAN "[%d]" RESET " %-20s | %-10s                      \n", 
                i + 1, hunters[i].username, status);
        }
    }
    printf("                                                                \n");
    
    char name[32];
    printf(" Enter hunter username to toggle ban: ");
    scanf("%s", name);
    getchar();
    
    int found = 0;
    for (int i = 0; i < MAX_HUNTER; i++) {
        if (hunters[i].used && strcmp(hunters[i].username, name) == 0) {
            hunters[i].banned = !hunters[i].banned;
            found = 1;
            printf("                                                                \n");
            printf(" " YELLOW "Hunter %s is now %s" RESET "                              \n", 
                   name, hunters[i].banned ? RED "BANNED" RESET : GREEN "UNBANNED" RESET);
            break;
        }
    }
    
    if (!found) {
        printf("                                                                \n");
        printf(" " RED "Hunter not found. Please check the username." RESET "               \n");
    }
    
    print_footer();
}

void reset_hunter_stats(HunterData *hunters) {
    printf(CLEAR_SCREEN);
    print_header(BLUE "RESET HUNTER STATS" YELLOW);

    printf(" " CYAN "Hunter List:" RESET "                                              \n");
    for (int i = 0; i < MAX_HUNTER; i++) {
        if (hunters[i].used) {
            char status[20];
            sprintf(status, "%s", hunters[i].banned ? RED "BANNED" RESET : GREEN "ACTIVE" RESET);
            printf(" " CYAN "[%d]" RESET " %-20s | %s | Lv:%2d | ATK:%3d | HP:%3d | DEF:%2d    \n", 
                i + 1, hunters[i].username, status, hunters[i].level, hunters[i].atk, hunters[i].hp, hunters[i].def);
        }
    }
    printf("                                                                \n");
    
    char name[32];
    printf(" Enter hunter username to reset: ");
    scanf("%s", name);
    getchar();
    
    int found = 0;
    for (int i = 0; i < MAX_HUNTER; i++) {
        if (hunters[i].used && strcmp(hunters[i].username, name) == 0) {
            printf("                                                                \n");
            printf(" " YELLOW "Before Reset:" RESET " Status: %s | Lv:%2d | ATK:%3d | HP:%3d | DEF:%2d             \n",
                  hunters[i].banned ? RED "BANNED" RESET : GREEN "ACTIVE" RESET,
                  hunters[i].level, hunters[i].atk, hunters[i].hp, hunters[i].def);
            
            hunters[i].level = 1;
            hunters[i].exp = 0;
            hunters[i].atk = 10;
            hunters[i].hp = 100;
            hunters[i].def = 5;

            if (hunters[i].banned) {
                hunters[i].banned = 0;
            }
            
            printf(" " GREEN "After Reset: " RESET " Status: %s | Lv:%2d | ATK:%3d | HP:%3d | DEF:%2d             \n",
                  GREEN "ACTIVE" RESET,
                  hunters[i].level, hunters[i].atk, hunters[i].hp, hunters[i].def);
            printf("                                                                \n");
            printf(" " GREEN "Stats for %s have been reset successfully." RESET "            \n", name);
            
            if (hunters[i].banned) {
                printf(" " GREEN "Hunter %s has also been unbanned." RESET "                 \n", name);
            }
            
            found = 1;
            break;
        }
    }
    
    if (!found) {
        printf("                                                                \n");
        printf(" " RED "Hunter not found. Please check the username." RESET "               \n");
    }
    
    print_footer();
}

void generate_dungeon(Dungeon *dungeons) {
    printf(CLEAR_SCREEN);
    print_header(MAGENTA "GENERATE NEW DUNGEON" YELLOW);
    
    char *names[] = {"Demon Castle", "Shadow Temple", "Ant Nest", "Dragon Lair", "Cursed Forest",
                    "Haunted Mansion", "Goblin Cave", "Mystic Shrine", "Pirate Bay", "Underworld"};
    char *difficulty[] = {"Easy", "Normal", "Hard", "Expert", "Master"};
    int idx = rand() % 10;  
    int diff = rand() % 5; 
    
    int available_slot = -1;
    for (int i = 0; i < MAX_DUNGEON; i++) {
        if (!dungeons[i].used) {
            available_slot = i;
            break;
        }
    }
    
    if (available_slot >= 0) {
        int i = available_slot;
        dungeons[i].used = 1;
        
        char full_name[64];
        snprintf(full_name, sizeof(full_name), "%s (%s)", names[idx], difficulty[diff]);
        strncpy(dungeons[i].name, full_name, sizeof(dungeons[i].name) - 1);
        dungeons[i].name[sizeof(dungeons[i].name) - 1] = '\0'; 

        dungeons[i].min_level = diff + 1;
        
        dungeons[i].reward_exp = (rand() % 101) + 100 + (diff * 50); 
        dungeons[i].reward_atk = (rand() % 31) + 10 + (diff * 20);    
        dungeons[i].reward_hp = (rand() % 51) + 50 + (diff * 20);    
        dungeons[i].reward_def = (rand() % 16) + 10 + (diff * 5);    

        printf(" " GREEN "⚔️  Dungeon Created Successfully! ⚔️" RESET "                       \n");
        printf("                                                                \n");
        printf(" " CYAN "Name:      " RESET "%s                              \n", dungeons[i].name);
        printf(" " YELLOW "Difficulty: " RESET "%s (" MAGENTA "Level %d" RESET " Required)                    \n", 
               difficulty[diff], dungeons[i].min_level);
        printf("                                                                \n");
        printf(" " GREEN "Rewards:" RESET "                                                   \n");
        printf("   🌟 EXP: " CYAN "+%d" RESET "                                              \n", dungeons[i].reward_exp);
        printf("   ⚔️  ATK: " RED "+%d" RESET "                                              \n", dungeons[i].reward_atk);
        printf("   ❤️  HP:  " GREEN "+%d" RESET "                                              \n", dungeons[i].reward_hp);
        printf("   🛡️  DEF: " BLUE "+%d" RESET "                                               \n", dungeons[i].reward_def);
    } else {
        printf(" " RED "No more space for dungeons! Maximum reached (%d)." RESET "         \n", MAX_DUNGEON);
        printf(" " YELLOW "Please wait until some hunters clear existing dungeons." RESET "    \n");
    }
    
    print_footer();
}

void show_all_dungeons(Dungeon *dungeons) {
    printf(CLEAR_SCREEN);
    print_header(MAGENTA "AVAILABLE DUNGEONS" YELLOW);

    int count = 0;
    for (int i = 0; i < MAX_DUNGEON; i++) {
        if (dungeons[i].used) {
            count++;
            printf(" " MAGENTA "[%d]" RESET " %-25s | Min Lv:%2d | EXP:%3d | ATK:+%2d | HP:+%3d | DEF:+%2d\n",
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

    if (count == 0) {
        printf(" " RED "No dungeons available yet..." RESET "\n");
        print_footer();
        return;
    }

    char opt;
    do {
        printf("\nDo you want to delete a dungeon? (y/n): ");
        scanf(" %c", &opt);
        getchar();

        if (opt == 'y' || opt == 'Y') {
            int del_idx;
            printf("Enter the dungeon number to delete: ");
            scanf("%d", &del_idx);
            getchar();

            if (del_idx < 1 || del_idx > MAX_DUNGEON || !dungeons[del_idx - 1].used) {
                printf(RED "Invalid dungeon number!\n" RESET);
            } else {
                printf(YELLOW "Deleting dungeon: %s...\n" RESET, dungeons[del_idx - 1].name);
                memset(&dungeons[del_idx - 1], 0, sizeof(Dungeon)); 
                dungeons[del_idx - 1].used = 0;
                printf(GREEN "Dungeon [%d] has been deleted.\n" RESET, del_idx);
            }
        }

        int still_available = 0;
        for (int i = 0; i < MAX_DUNGEON; i++) {
            if (dungeons[i].used) {
                still_available = 1;
                break;
            }
        }

        if (!still_available) {
            printf(RED "No more dungeons available to delete.\n" RESET);
            break;
        }

    } while (opt == 'y' || opt == 'Y');

    print_footer();
}
