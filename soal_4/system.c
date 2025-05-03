#include "shm_common.h"
#include <errno.h>

#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"

#define CLEAR_SCREEN "\033[2J\033[H"

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

void handle_signal(int sig) {
    printf("\nReceived signal %d. Cleaning up shared memory...\n", sig);
    cleanup_shared_memory();
    exit(0);
}

void show_all_hunters(struct SystemData *data);
void ban_or_unban(struct SystemData *data);
void reset_hunter_stats(struct SystemData *data);
void generate_dungeon(struct SystemData *data);
void show_all_dungeons(struct SystemData *data);
void print_header(const char *title);
void print_footer();
void initialize_memory(struct SystemData *data);

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

void initialize_memory(struct SystemData *data) {
    if (data->num_hunters == 0 && data->num_dungeons == 0) {
        printf(YELLOW "Initializing shared memory for first use...\n" RESET);
        
        for (int i = 0; i < MAX_HUNTERS; i++) {
            memset(&data->hunters[i], 0, sizeof(struct Hunter));
        }
        for (int i = 0; i < MAX_DUNGEONS; i++) {
            memset(&data->dungeons[i], 0, sizeof(struct Dungeon));
        }
        data->num_hunters = 0;
        data->num_dungeons = 0;
        data->current_notification_index = 0;
        
        printf(GREEN "Memory initialization complete!\n" RESET);
    } else {
        printf(GREEN "Using existing shared memory data.\n" RESET);
    }
}

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
                i + 1,
                data->hunters[i].username,
                data->hunters[i].level,
                data->hunters[i].exp,
                data->hunters[i].atk,
                data->hunters[i].hp,
                data->hunters[i].def,
                status
            );
        }
    }
    
    print_footer();
}

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
    printf("                                                                \n");
    
    char name[50];
    printf(" Enter hunter username to toggle ban: ");
    scanf("%s", name);
    getchar();
    
    int found = 0;
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, name) == 0) {
            data->hunters[i].banned = !data->hunters[i].banned;
            found = 1;
            printf("                                                                \n");
            printf(" " YELLOW "Hunter %s is now %s" RESET "                              \n", 
                   name, data->hunters[i].banned ? RED "BANNED" RESET : GREEN "UNBANNED" RESET);
            break;
        }
    }
    
    if (!found) {
        printf("                                                                \n");
        printf(" " RED "Hunter not found. Please check the username." RESET "               \n");
    }
    
    print_footer();
}

void reset_hunter_stats(struct SystemData *data) {
    printf(CLEAR_SCREEN);
    print_header(BLUE "RESET HUNTER STATS" YELLOW);

    printf(" " CYAN "Hunter List:" RESET "                                              \n");
    for (int i = 0; i < data->num_hunters; i++) {
        char status[20];
        sprintf(status, "%s", data->hunters[i].banned ? RED "BANNED" RESET : GREEN "ACTIVE" RESET);
        printf(" " CYAN "[%d]" RESET " %-20s | %s | Lv:%2d | ATK:%3d | HP:%3d | DEF:%2d    \n", 
            i + 1, data->hunters[i].username, status, data->hunters[i].level, 
            data->hunters[i].atk, data->hunters[i].hp, data->hunters[i].def);
    }
    printf("                                                                \n");
    
    char name[50];
    printf(" Enter hunter username to reset: ");
    scanf("%s", name);
    getchar();
    
    int found = 0;
    for (int i = 0; i < data->num_hunters; i++) {
        if (strcmp(data->hunters[i].username, name) == 0) {
            printf("                                                                \n");
            printf(" " YELLOW "Before Reset:" RESET " Status: %s | Lv:%2d | ATK:%3d | HP:%3d | DEF:%2d             \n",
                  data->hunters[i].banned ? RED "BANNED" RESET : GREEN "ACTIVE" RESET,
                  data->hunters[i].level, data->hunters[i].atk, data->hunters[i].hp, data->hunters[i].def);
            
            data->hunters[i].level = 1;
            data->hunters[i].exp = 0;
            data->hunters[i].atk = 10;
            data->hunters[i].hp = 100;
            data->hunters[i].def = 5;
            data->hunters[i].banned = 0;
            
            printf(" " GREEN "After Reset: " RESET " Status: %s | Lv:%2d | ATK:%3d | HP:%3d | DEF:%2d             \n",
                  data->hunters[i].banned ? RED "BANNED" RESET : GREEN "ACTIVE" RESET,
                  data->hunters[i].level, data->hunters[i].atk, data->hunters[i].hp, data->hunters[i].def);
            printf(" " GREEN "Stats for %s have been reset successfully." RESET "            \n", name);
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
        printf(" " YELLOW "Please wait until some hunters clear existing dungeons." RESET "    \n");
        print_footer();
        return;
    }
    
    struct Dungeon *new_dungeon = &data->dungeons[data->num_dungeons];
    char full_name[50];
    snprintf(full_name, sizeof(full_name), "%s (%s)", names[idx], difficulty[diff]);
    strncpy(new_dungeon->name, full_name, sizeof(new_dungeon->name) - 1);
    new_dungeon->name[sizeof(new_dungeon->name) - 1] = '\0'; 

    new_dungeon->min_level = diff + 1;
    new_dungeon->exp = (rand() % 101) + 100 + (diff * 50); 
    new_dungeon->atk = (rand() % 31) + 10 + (diff * 20);    
    new_dungeon->hp = (rand() % 51) + 50 + (diff * 20);    
    new_dungeon->def = (rand() % 16) + 10 + (diff * 5);    
    new_dungeon->shm_key = get_system_key();

    data->num_dungeons++;

    printf(" " GREEN "⚔️  Dungeon Created Successfully! ⚔️" RESET "                       \n");
    printf("                                                                \n");
    printf(" " CYAN "Name:      " RESET "%s                              \n", new_dungeon->name);
    printf(" " YELLOW "Difficulty: " RESET "%s (" MAGENTA "Level %d" RESET " Required)                    \n", 
           difficulty[diff], new_dungeon->min_level);
    printf("                                                                \n");
    printf(" " GREEN "Rewards:" RESET "                                                   \n");
    printf("   🌟 EXP: " CYAN "+%d" RESET "                                              \n", new_dungeon->exp);
    printf("   ⚔️  ATK: " RED "+%d" RESET "                                              \n", new_dungeon->atk);
    printf("   ❤️  HP:  " GREEN "+%d" RESET "                                              \n", new_dungeon->hp);
    printf("   🛡️  DEF: " BLUE "+%d" RESET "                                               \n", new_dungeon->def);
    
    print_footer();
}

void show_all_dungeons(struct SystemData *data) {
    printf(CLEAR_SCREEN);
    print_header(MAGENTA "AVAILABLE DUNGEONS" YELLOW);

    if (data->num_dungeons == 0) {
        printf(" " RED "No dungeons available yet..." RESET "\n");
        print_footer();
        return;
    }

    for (int i = 0; i < data->num_dungeons; i++) {
        printf(" " MAGENTA "[%d]" RESET " %-25s | Min Lv:%2d | EXP:%3d | ATK:+%2d | HP:+%3d | DEF:+%2d\n",
            i + 1,
            data->dungeons[i].name,
            data->dungeons[i].min_level,
            data->dungeons[i].exp,
            data->dungeons[i].atk,
            data->dungeons[i].hp,
            data->dungeons[i].def
        );
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

int main() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal); 

    srand(time(NULL));

    printf(CLEAR_SCREEN);
    printf(YELLOW);
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("                   " CYAN "HUNTER MANAGEMENT SYSTEM" YELLOW "                   \n");
    printf("                " MAGENTA "Created by Sung Jin Woo" YELLOW "                    \n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf(RESET);
    
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
            case 1: show_all_hunters(data); break;
            case 2: show_all_dungeons(data); break;
            case 3: generate_dungeon(data); break;
            case 4: ban_or_unban(data); break;
            case 5: reset_hunter_stats(data); break;
            case 6:
                printf(CLEAR_SCREEN);
                printf(YELLOW);
                printf("╔════════════════════════════════════════════════════════════════╗\n");
                printf("                    " RED "SHUTTING DOWN SYSTEM" YELLOW "                    \n");
                printf("╚════════════════════════════════════════════════════════════════╝\n");
                printf(RESET);
                cleanup_shared_memory();
                printf(GREEN "System shutdown complete. Shared memory deleted. Goodbye!\n" RESET);
                exit(0);
            default:
                printf(RED "Invalid choice! Try again.\n" RESET);
                sleep(1);
        }
    }
}
