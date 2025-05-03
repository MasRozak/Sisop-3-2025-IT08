// shop.c
#include <stdio.h>
#include <string.h>
#include "shop.h"

// ANSI color codes
#define ANSI_RED     "\033[1;31m"
#define ANSI_GREEN   "\033[1;32m"
#define ANSI_YELLOW  "\033[1;33m"
#define ANSI_BLUE    "\033[1;34m"
#define ANSI_PURPLE  "\033[1;35m"
#define ANSI_CYAN    "\033[1;36m"
#define ANSI_RESET   "\033[0m"

// Global array of weapons
Weapon shop_weapons[WEAPON_COUNT];

// Initialize shop with weapons
void init_shop() {
    shop_weapons[0] = (Weapon){1, "Terra Blade", 50, 10, ""};
    shop_weapons[1] = (Weapon){2, "Flint & Steel", 150, 25, ""};
    shop_weapons[2] = (Weapon){3, "Kitchen Knife", 200, 35, ""};
    shop_weapons[3] = (Weapon){4, "Staff of Light", 120, 20, "10% Insta-Kill Chance"};
    shop_weapons[4] = (Weapon){5, "Dragon Claws", 300, 50, "30% Crit Chance"};
}

// Get weapon by ID
Weapon* get_weapon_by_id(int id) {
    for (int i = 0; i < WEAPON_COUNT; i++) {
        if (shop_weapons[i].id == id) {
            return &shop_weapons[i];
        }
    }
    return NULL;
}

// Process weapon purchase
int buy_weapon(Player *p, int weapon_id) {
    Weapon *w = get_weapon_by_id(weapon_id);
    if (!w) {
        return -1; // Invalid weapon ID
    }
    if (p->gold < w->price) {
        return -2; // Not enough gold
    }
    for (int i = 0; i < p->inventory_size; i++) {
        if (p->inventory[i] == weapon_id) {
            return -3; // Already owned
        }
    }
    if (p->inventory_size >= WEAPON_COUNT) {
        return -4; // Inventory full
    }
    // Deduct gold and add weapon to inventory
    p->gold -= w->price;
    p->inventory[p->inventory_size++] = weapon_id;
    // Equip the weapon
    strcpy(p->equipped_weapon, w->name);
    p->base_damage = w->damage;
    return 0; // Success
}

// Server-side shop logic (for GET_SHOP response)
void shop(Player *p, char *response, size_t response_size) {
    response[0] = '\0';
    char line[200]; // Tingkatkan ukuran buffer untuk menghindari pemotongan
    for (int i = 0; i < WEAPON_COUNT; i++) {
        Weapon *w = &shop_weapons[i];
        line[0] = '\0';
        if (strlen(w->passive) > 0) {
            snprintf(line, sizeof(line), "[%d] %s - %d gold, %d damage (Passive: %s)\n",
                     w->id, w->name, w->price, w->damage, w->passive);
        } else {
            snprintf(line, sizeof(line), "[%d] %s - %d gold, %d damage\n",
                     w->id, w->name, w->price, w->damage);
        }
        strncat(response, line, response_size - strlen(response) - 1);
    }
}

// Display shop for local mode with ASCII art
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
