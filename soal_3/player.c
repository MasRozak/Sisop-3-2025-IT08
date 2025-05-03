#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "shop.h"

#define ANSI_RED     "\033[1;31m"
#define ANSI_GREEN   "\033[1;32m"
#define ANSI_YELLOW  "\033[1;33m"
#define ANSI_BLUE    "\033[1;34m"
#define ANSI_PURPLE  "\033[1;35m"
#define ANSI_CYAN    "\033[1;36m"
#define ANSI_RESET   "\033[0m"

int is_local = 0;
int client_sock = -1;

void init_client(char *host, int port) {
    struct sockaddr_in server_addr;
    struct addrinfo hints, *res;

    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (strcmp(host, "localhost") == 0) {
        host = "127.0.0.1";
    }

    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);
    if (getaddrinfo(host, port_str, &hints, &res) != 0) {
        fprintf(stderr, ANSI_RED "⚠ Failed to resolve address: %s\n" ANSI_RESET, host);
        close(client_sock);
        exit(1);
    }

    memcpy(&server_addr, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror(ANSI_RED "⚠ Connection to server failed" ANSI_RESET);
        close(client_sock);
        exit(1);
    }
    printf(ANSI_CYAN "🌌 Connected to server at %s:%d\n" ANSI_RESET, host, port);
}

void send_command(char *command, char *response, size_t response_size) {
    if (write(client_sock, command, strlen(command)) < 0) {
        perror(ANSI_RED "⚠ Failed to send command" ANSI_RESET);
        return;
    }
    memset(response, 0, response_size);
    int n = read(client_sock, response, response_size - 1);
    if (n > 0) {
        response[n] = '\0';
    } else if (n < 0) {
        perror(ANSI_RED "⚠ Failed to read response" ANSI_RESET);
    }
}

void show_player_stats(Player *p) {
    char command[100], response[MAX_MESSAGE * 2];
    snprintf(command, sizeof(command), "GET_STATS %s", p->client_id);
    send_command(command, response, sizeof(response));

    int gold = 0, damage = 0, kills = 0;
    char weapon[50] = "", passive[100] = "";
    if (sscanf(response, "Gold:%d;Weapon:%[^;];Damage:%d;Kills:%d;Passive:%[^\n]", 
               &gold, weapon, &damage, &kills, passive) < 4) {
        printf(ANSI_RED "⚠ Failed to parse stats\n" ANSI_RESET);
        return;
    }

    printf(ANSI_CYAN "┌────────────────────────────────────┐\n" ANSI_RESET);
    printf(ANSI_CYAN " " ANSI_YELLOW "         ⚔ HERO'S JOURNAL ⚔         " ANSI_CYAN " \n" ANSI_RESET);
    printf(ANSI_CYAN "├────────────────────────────────────┤\n" ANSI_RESET);
    printf(ANSI_CYAN "  " ANSI_YELLOW "Gold: %-5d 💰" ANSI_RESET "                   " ANSI_CYAN "   \n", gold);
    printf(ANSI_CYAN "  " ANSI_GREEN "Weapon: %-20s" ANSI_RESET " " ANSI_CYAN "       \n", weapon);
    printf(ANSI_CYAN "  " ANSI_RED "Damage: %-3d ⚡" ANSI_RESET "                 " ANSI_CYAN "     \n", damage);
    printf(ANSI_CYAN "  " ANSI_BLUE "Kills: %-4d ☠" ANSI_RESET "                  " ANSI_CYAN "     \n", kills);
    if (strlen(passive) > 0) {
        printf(ANSI_CYAN "  " ANSI_PURPLE "Passive: %-19s" ANSI_RESET " " ANSI_CYAN " \n", passive);
    }
    printf(ANSI_CYAN "└────────────────────────────────────┘\n\n" ANSI_RESET);
}

void client_shop(Player *p) {
    char input[10];
    int choice;

    printf(ANSI_PURPLE "┌───────────────────────────────────────────────────────────────┐\n" ANSI_RESET);
    printf(ANSI_PURPLE " " ANSI_YELLOW "         🛒 MYSTERIOUS ARMORY 🛒             " ANSI_PURPLE " \n" ANSI_RESET);
    printf(ANSI_PURPLE "├────┬──────────────────┬──────┬──────┬─────────────────────────┤\n" ANSI_RESET);
    printf(ANSI_PURPLE " " ANSI_CYAN " ID " ANSI_PURPLE "│" ANSI_CYAN " Name             " ANSI_PURPLE "│" ANSI_CYAN " Gold " ANSI_PURPLE "│" ANSI_CYAN " Dmg  " ANSI_PURPLE "│" ANSI_CYAN " Passive" ANSI_PURPLE " \n" ANSI_RESET);
    printf(ANSI_PURPLE "├────┼──────────────────┼──────┼──────┼─────────────────────────┤\n" ANSI_RESET);

    char command[100], response[MAX_MESSAGE * 2];
    snprintf(command, sizeof(command), "GET_SHOP %s", p->client_id);
    send_command(command, response, sizeof(response));

    char *line = strtok(response, "\n");
    while (line) {
        int id, gold, damage;
        char name[50], passive[100] = "";
        if (sscanf(line, "[%d] %[^-] - %d gold, %d damage (Passive: %[^)])", 
                   &id, name, &gold, &damage, passive) >= 4) {
            printf(ANSI_PURPLE " " ANSI_CYAN " %-2d " ANSI_PURPLE "│" ANSI_GREEN " %-16s " ANSI_PURPLE "│" ANSI_YELLOW " %-4d " ANSI_PURPLE "│" ANSI_RED " %-4d " ANSI_PURPLE "│" ANSI_BLUE " %-6s " ANSI_PURPLE " \n", 
                   id, name, gold, damage, passive);
        } else if (sscanf(line, "[%d] %[^-] - %d gold, %d damage", 
                          &id, name, &gold, &damage) == 4) {
            printf(ANSI_PURPLE " " ANSI_CYAN " %-2d " ANSI_PURPLE "│" ANSI_GREEN " %-16s " ANSI_PURPLE "│" ANSI_YELLOW " %-4d " ANSI_PURPLE "│" ANSI_RED " %-4d " ANSI_PURPLE "│" ANSI_BLUE " %-6s " ANSI_PURPLE " \n", 
                   id, name, gold, damage, passive);
        }
        line = strtok(NULL, "\n");
    }
    printf(ANSI_PURPLE "└────┴──────────────────┴──────┴──────┴─────────────────────────┘\n" ANSI_RESET);
    printf(ANSI_YELLOW "💬 The merchant grins: 'Choose wisely, adventurer!'\n" ANSI_RESET);
    printf(ANSI_CYAN "➤ Enter weapon number to buy (0 to leave): " ANSI_RESET);

    fgets(input, sizeof(input), stdin);
    sscanf(input, "%d", &choice);

    if (choice == 0) {
        printf(ANSI_YELLOW "💬 The merchant nods: 'Come back anytime!'\n" ANSI_RESET);
        return;
    }
    if (choice < 1 || choice > WEAPON_COUNT) {
        printf(ANSI_RED "⚠ The merchant frowns: 'That's not on the list!'\n" ANSI_RESET);
        return;
    }

    snprintf(command, sizeof(command), "BUY_WEAPON %s %d", p->client_id, choice);
    send_command(command, response, sizeof(response));
    if (strstr(response, "successfully")) {
        printf(ANSI_GREEN "✅ %s" ANSI_RESET, response);
    } else {
        printf(ANSI_RED "⚠ %s" ANSI_RESET, response);
    }
}

void view_inventory(Player *p) {
    char input[10];
    int choice;

    printf(ANSI_BLUE "┌────────────────────────────────────────────────────┐\n" ANSI_RESET);
    printf(ANSI_BLUE "│" ANSI_YELLOW "                 🎒 HERO'S SATCHEL 🎒            " ANSI_BLUE "   │\n" ANSI_RESET);
    printf(ANSI_BLUE "├────┬──────────────────┬────────────────────────────┤\n" ANSI_RESET);
    printf(ANSI_BLUE " " ANSI_CYAN " ID " ANSI_BLUE "│" ANSI_CYAN " Name             " ANSI_BLUE "│" ANSI_CYAN " Status     " ANSI_BLUE "  \n" ANSI_RESET);
    printf(ANSI_BLUE "├────┼──────────────────┼────────────────────────────┤\n" ANSI_RESET);

    char command[100], response[MAX_MESSAGE * 2];
    snprintf(command, sizeof(command), "GET_INVENTORY %s", p->client_id);
    send_command(command, response, sizeof(response));

    char *line = strtok(response, "\n");
    while (line) {
        int id;
        char name[50], status[100] = "";
        if (strstr(line, ";EQUIPPED")) {
            sscanf(line, "[%d] %[^;];EQUIPPED", &id, name);
            strcpy(status, "EQUIPPED");
        } else if (sscanf(line, "[%d] %[^;];Passive:%[^;]", &id, name, status) == 3) {
        } else if (sscanf(line, "[%d] %[^;]", &id, name) == 2) {
            strcpy(status, "");
        }
        printf(ANSI_BLUE " " ANSI_CYAN " %-2d " ANSI_BLUE "│" ANSI_GREEN " %-16s " ANSI_BLUE "│" ANSI_YELLOW "%-12s" ANSI_BLUE " \n", 
               id, name, status);
        line = strtok(NULL, "\n");
    }
    printf(ANSI_BLUE "└────┴──────────────────┴────────────────────────────┘\n" ANSI_RESET);
    printf(ANSI_CYAN "➤ Enter number to equip (0 to cancel): " ANSI_RESET);

    fgets(input, sizeof(input), stdin);
    sscanf(input, "%d", &choice);

    if (choice == 0) return;
    snprintf(command, sizeof(command), "EQUIP %s %d", p->client_id, choice);
    send_command(command, response, sizeof(response));
    if (strstr(response, "equipped")) {
        printf(ANSI_GREEN "✅ %s" ANSI_RESET, response);
    } else {
        printf(ANSI_RED "⚠ %s" ANSI_RESET, response);
    }
}

void battle_mode(Player *p) {
    char input[10];

    printf(ANSI_RED "┌────────────────────────────────────┐\n" ANSI_RESET);
    printf(ANSI_RED "│" ANSI_YELLOW "        ⚡ THE DARK GATE ⚡         " ANSI_RED "│\n" ANSI_RESET);
    printf(ANSI_RED "└────────────────────────────────────┘\n" ANSI_RESET);

    while (1) {
        char command[100], response[MAX_MESSAGE * 2];
        printf(ANSI_CYAN "\n➤ Type '" ANSI_GREEN "attack" ANSI_CYAN "' or '" ANSI_RED "exit" ANSI_CYAN "': " ANSI_RESET);
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;
        snprintf(command, sizeof(command), "BATTLE %s %s", p->client_id, input);
        send_command(command, response, sizeof(response));

        if (strstr(response, "Enemy HP:")) {
            int current_hp, max_hp;
            char *hp_line = strstr(response, "Enemy HP:");
            if (hp_line && sscanf(hp_line, "Enemy HP: %d/%d", &current_hp, &max_hp) == 2) {
                char *msg = strtok(response, "\n");
                while (msg && msg < hp_line) {
                    if (strstr(msg, "CRITICAL HIT")) {
                        printf(ANSI_RED "💥 %s" ANSI_RESET, msg);
                    } else if (strstr(msg, "INSTANT KILL")) {
                        printf(ANSI_PURPLE "✨ %s" ANSI_RESET, msg);
                    } else if (strstr(msg, "Enemy defeated")) {
                        printf(ANSI_YELLOW "🎉 %s" ANSI_RESET, msg);
                    } else {
                        printf(ANSI_GREEN "⚔ %s" ANSI_RESET, msg);
                    }
                    msg = strtok(NULL, "\n");
                }
                printf(ANSI_RED "👾 Enemy status:\n" ANSI_RESET);
                int bar_width = 30;
                int filled = (current_hp * bar_width) / max_hp;
                printf(ANSI_RED "❤️ [" ANSI_GREEN);
                if (current_hp < max_hp / 3) printf(ANSI_RED);
                for (int i = 0; i < filled; i++) printf("█");
                printf(ANSI_RESET);
                for (int i = filled; i < bar_width; i++) printf(" ");
                printf(ANSI_RED "] %d/%d HP\n" ANSI_RESET, current_hp, max_hp);
            }
        } else {
            if (strstr(response, "appears")) {
                printf(ANSI_YELLOW "👹 %s" ANSI_RESET, response);
            } else if (strstr(response, "fled")) {
                printf(ANSI_YELLOW "🏃 %s" ANSI_RESET, response);
            } else if (strstr(response, "Invalid")) {
                printf(ANSI_RED "⚠ %s" ANSI_RESET, response);
            } else {
                printf("%s", response);
            }
        }
        if (strcmp(input, "exit") == 0) return;
    }
}

void main_menu(Player *p) {
    char input[10];
    int choice;

    while (1) {
        printf(ANSI_GREEN "┌────────────────────────────────────┐\n" ANSI_RESET);
        printf(ANSI_GREEN "│" ANSI_YELLOW "       🏰 LOST DUNGEON 🏰           " ANSI_GREEN "│\n" ANSI_RESET);
        printf(ANSI_GREEN "├────────────────────────────────────┤\n" ANSI_RESET);
        printf(ANSI_GREEN "│ " ANSI_CYAN "[1] Hero's Journal (Stats)   📜" ANSI_RESET " " ANSI_GREEN "   │\n" ANSI_RESET);
        printf(ANSI_GREEN "│ " ANSI_CYAN "[2] Mysterious Armory (Shop) 🛒" ANSI_RESET " " ANSI_GREEN "   │\n" ANSI_RESET);
        printf(ANSI_GREEN "│ " ANSI_CYAN "[3] Hero's Satchel (Inventory)🎒" ANSI_RESET " " ANSI_GREEN "  │\n" ANSI_RESET);
        printf(ANSI_GREEN "│ " ANSI_CYAN "[4] The Dark Gate (Battle)   ⚔" ANSI_RESET " " ANSI_GREEN "    │\n" ANSI_RESET);
        printf(ANSI_GREEN "│ " ANSI_CYAN "[5] Flee the Dungeon (Exit)  🏃" ANSI_RESET " " ANSI_GREEN "   │\n" ANSI_RESET);
        printf(ANSI_GREEN "└────────────────────────────────────┘\n" ANSI_RESET);
        printf(ANSI_YELLOW "➤ Choose your path: " ANSI_RESET);

        fgets(input, sizeof(input), stdin);
        sscanf(input, "%d", &choice);

        switch (choice) {
            case 1:
                show_player_stats(p);
                break;
            case 2:
                client_shop(p);
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

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "local") == 0) {
        fprintf(stderr, ANSI_RED "⚠ Local mode not supported in client. Run ./dungeon local instead.\n" ANSI_RESET);
        exit(1);
    } else if (argc > 2) {
        init_client(argv[1], 8080);
        Player client_player;
        snprintf(client_player.client_id, sizeof(client_player.client_id), "%s", argv[2]);
        main_menu(&client_player);
        close(client_sock);
    } else {
        fprintf(stderr, ANSI_RED "⚠ Usage: %s <server_host> <client_id>\n" ANSI_RESET, argv[0]);
        exit(1);
    }

    return 0;
}
