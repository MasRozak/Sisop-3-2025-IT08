#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include "shop.h"

#define ANSI_RED     "\033[1;31m"
#define ANSI_GREEN   "\033[1;32m"
#define ANSI_YELLOW  "\033[1;33m"
#define ANSI_BLUE    "\033[1;34m"
#define ANSI_PURPLE  "\033[1;35m"
#define ANSI_CYAN    "\033[1;36m"
#define ANSI_RESET   "\033[0m"

Player sessions[MAX_CLIENTS];
int session_count = 0;
pthread_mutex_t session_mutex = PTHREAD_MUTEX_INITIALIZER;

Player* get_player_session(char *client_id) {
    pthread_mutex_lock(&session_mutex);
    for (int i = 0; i < session_count; i++) {
        if (strcmp(sessions[i].client_id, client_id) == 0) {
            pthread_mutex_unlock(&session_mutex);
            return &sessions[i];
        }
    }
    if (session_count < MAX_CLIENTS) {
        Player *p = &sessions[session_count];
        snprintf(p->client_id, sizeof(p->client_id), "%s", client_id);
        p->gold = 500;
        strcpy(p->equipped_weapon, "Fists");
        p->base_damage = 5;
        p->kills = 0;
        p->inventory_size = 0;
        session_count++;
        pthread_mutex_unlock(&session_mutex);
        return p;
    }
    pthread_mutex_unlock(&session_mutex);
    return NULL;
}

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

static void print_health_bar(int current, int max) {
    int bar_width = 30;
    int filled = (current * bar_width) / max;
    printf(ANSI_RED "❤️ [" ANSI_GREEN);
    if (current < max / 3) printf(ANSI_RED);
    for (int i = 0; i < filled; i++) printf("█");
    printf(ANSI_RESET);
    for (int i = filled; i < bar_width; i++) printf(" ");
    printf(ANSI_RED "] %d/%d HP\n" ANSI_RESET, current, max);
}

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
            case 1:
                show_player_stats(p);
                break;
            case 2:
                display_shop(p); 
                break;
            case 3:
                view_inventory(p);
                break;
            case 4:
                battle_mode(p);
                break;
            case 5:
                printf(ANSI_YELLOW "🏃 You escape the dungeon's grasp... for now.\n" ANSI_RESET);
                return;
            default:
                printf(ANSI_RED "⚠ The dungeon whispers: 'Choose wisely!'\n" ANSI_RESET);
                break;
        }
    }
}

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

        if (strcmp(command, "GET_STATS") == 0) {
            char passive[100] = "";
            for (int i = 0; i < WEAPON_COUNT; i++) {
                if (strcmp(p->equipped_weapon, shop_weapons[i].name) == 0 && strlen(shop_weapons[i].passive) > 0) {
                    strcpy(passive, shop_weapons[i].passive); 
                    break;
                }
            }
            snprintf(response, sizeof(response), 
                     "Gold:%d;Weapon:%s;Damage:%d;Kills:%d;Passive:%s\n",
                     p->gold, p->equipped_weapon, p->base_damage, p->kills, passive);
        } else if (strcmp(command, "GET_SHOP") == 0) {
            shop(p, response, sizeof(response));
        } else if (strcmp(command, "BUY_WEAPON") == 0) {
            int weapon_id;
            sscanf(buffer, "%s %s %d", command, param, &weapon_id);
            int result = buy_weapon(p, weapon_id);
            if (result == 0) {
                snprintf(response, sizeof(response), "Successfully bought %s!\n", p->equipped_weapon);
            } else if (result == -1) {
                snprintf(response, sizeof(response), "Invalid weapon ID!\n");
            } else if (result == -2) {
                snprintf(response, sizeof(response), "Not enough gold!\n");
            } else if (result == -3) {
                snprintf(response, sizeof(response), "You already own this weapon!\n");
            } else {
                snprintf(response, sizeof(response), "Inventory full!\n");
            }
        } else if (strcmp(command, "GET_INVENTORY") == 0) {
            strcpy(response, "");
            snprintf(response, sizeof(response), "[1] Fists%s\n", 
                     strcmp(p->equipped_weapon, "Fists") == 0 ? ";EQUIPPED" : "");
            for (int i = 0; i < p->inventory_size; i++) {
                Weapon *w = get_weapon_by_id(p->inventory[i]);
                if (w) {
                    snprintf(response + strlen(response), sizeof(response) - strlen(response),
                             "[%d] %s", w->id, w->name);
                    if (strlen(w->passive) > 0) {
                        snprintf(response + strlen(response), sizeof(response) - strlen(response),
                                 ";Passive:%s", w->passive);
                    }
                    if (strcmp(p->equipped_weapon, w->name) == 0) {
                        snprintf(response + strlen(response), sizeof(response) - strlen(response),
                                 ";EQUIPPED");
                    }
                    strcat(response, "\n");
                }
            }
        } else if (strcmp(command, "EQUIP") == 0) {
            int choice;
            sscanf(buffer, "%s %s %d", command, param, &choice);
            if (choice == 1) {
                strcpy(p->equipped_weapon, "Fists");
                p->base_damage = 5;
                snprintf(response, sizeof(response), "Fists equipped.\n");
            } else if (choice >= 2 && choice <= WEAPON_COUNT + 1) {
                Weapon *w = get_weapon_by_id(choice - 1);
                if (w) {
                    int owned = 0;
                    for (int i = 0; i < p->inventory_size; i++) {
                        if (p->inventory[i] == w->id) {
                            owned = 1;
                            strcpy(p->equipped_weapon, w->name);
                            p->base_damage = w->damage;
                            snprintf(response, sizeof(response), "%s equipped!\n", w->name);
                            break;
                        }
                    }
                    if (!owned) {
                        snprintf(response, sizeof(response), "You don't own this weapon.\n");
                    }
                } else {
                    snprintf(response, sizeof(response), "Invalid weapon.\n");
                }
            } else {
                snprintf(response, sizeof(response), "Invalid choice.\n");
            }
        } else if (strcmp(command, "BATTLE") == 0) {
            char action[10];
            sscanf(buffer, "%s %s %s", command, param, action);

            if (strcmp(action, "attack") == 0) {
                if (enemy_hp <= 0) {
                    enemy_max_hp = rand() % 151 + 50;
                    enemy_hp = enemy_max_hp;
                    snprintf(response, sizeof(response), "A shadowy beast appears!\nEnemy HP: %d/%d\n", enemy_hp, enemy_max_hp);
                } else {
                    strcpy(response, "");
                    int damage = p->base_damage + (rand() % 6 + 1);
                    int is_crit = (rand() % 100 < 10);
                    if (strcmp(p->equipped_weapon, "Dragon Claws") == 0) {
                        is_crit = (rand() % 100 < 30);
                    } else if (strcmp(p->equipped_weapon, "Staff of Light") == 0) {
                        if (rand() % 100 < 10) {
                            damage = enemy_hp;
                            snprintf(response, sizeof(response), "STAFF OF LIGHT INSTANT KILL!\n");
                        }
                    }
                    if (is_crit && strlen(response) == 0) {
                        damage *= 2;
                        snprintf(response, sizeof(response), "CRITICAL HIT! Dealt %d damage!\n", damage);
                    } else if (strlen(response) == 0) {
                        snprintf(response, sizeof(response), "Dealt %d damage!\n", damage);
                    }
                    enemy_hp -= damage;
                    if (enemy_hp <= 0) {
                        int gold_reward = rand() % 101 + 50;
                        p->gold += gold_reward;
                        p->kills++;
                        snprintf(response + strlen(response), sizeof(response) - strlen(response),
                                 "Enemy defeated! Earned %d gold!\n", gold_reward);
                        enemy_hp = 0;
                    } else {
                        snprintf(response + strlen(response), sizeof(response) - strlen(response),
                                 "Enemy HP: %d/%d\n", enemy_hp, enemy_max_hp);
                    }
                }
            } else if (strcmp(action, "exit") == 0) {
                snprintf(response, sizeof(response), "You fled the battle.\n");
                enemy_hp = 0;
            } else {
                snprintf(response, sizeof(response), "Invalid command.\n");
            }
        } else {
            snprintf(response, sizeof(response), "Unknown command.\n");
        }

        write(client_sock, response, strlen(response));
    }

    close(client_sock);
    return NULL;
}

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
