// Simple game logic only - no inline assembly
// I/O will be handled by bootloader

void print_char(char c) {
    asm volatile (
        "mov $0x0e, %%ah\n"
        "mov %0, %%al\n"
        "int $0x10"
        :
        : "r" (c)
        : "ah", "al"
    );
}

void print_string(char* str) {
    while (*str) {
        print_char(*str);
        str++;
    }
}

char read_key() {
    char key;
    asm volatile (
        "mov $0x00, %%ah\n"
        "int $0x16\n"
        "mov %%al, %0"
        : "=r" (key)
        :
        : "ah", "al"
    );
    return key;
}

void clear_screen() {
    asm volatile (
        "mov $0x00, %%ah\n"
        "mov $0x03, %%al\n"
        "int $0x10"
        :
        :
        : "ah", "al"
    );
}

int player_cards[10];
int dealer_cards[10];
int player_count = 0;
int dealer_count = 0;
unsigned int seed = 1;

// Card encoding: lower 4 bits = value (1-13), upper 4 bits = suit (0-3)
// Values: 1=A, 2-10=face value, 11=J, 12=Q, 13=K
// Suits: 0=♣, 1=♥, 2=♦, 3=♠
int deck[52] = {
    // Clubs ♣ (suit 0)
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
    // Hearts ♥ (suit 1)
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
    // Diamonds ♦ (suit 2)
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,
    // Spades ♠ (suit 3)
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D
};

// Helper functions for card handling
int get_card_value(int card) {
    return card & 0x0F;  // Lower 4 bits
}

int get_card_suit(int card) {
    return (card >> 4) & 0x0F;  // Upper 4 bits
}

int get_blackjack_value(int card) {
    int value = get_card_value(card);
    if (value >= 11 && value <= 13) {  // J, Q, K
        return 10;
    }
    return value;  // A=1, 2-10=face value
}

void print_suit(int suit) {
    switch(suit) {
        case 0: print_char(5); break;   // ♣ Clubs (ASCII 5)
        case 1: print_char(3); break;   // ♥ Hearts (ASCII 3)
        case 2: print_char(4); break;   // ♦ Diamonds (ASCII 4)
        case 3: print_char(6); break;   // ♠ Spades (ASCII 6)
        default: print_char('?'); break;
    }
}

void print_card_value(int value) {
    switch(value) {
        case 1: print_char('A'); break;
        case 10: print_string("10"); break;
        case 11: print_char('J'); break;
        case 12: print_char('Q'); break;
        case 13: print_char('K'); break;
        default: print_char('0' + value); break;
    }
}

// Simple pseudo-random number generator
int random() {
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
}

int give_card() {
    int pos = random() % 52;
    return deck[pos];
}

int calculate_hand(int* cards, int count) {
    int total = 0;
    int aces = 0;

    for (int i = 0; i < count; i++) {
        int value = get_card_value(cards[i]);
        if (value == 1) {  // Ace
            aces++;
            total += 11;
        } else if (value >= 11 && value <= 13) {  // J, Q, K
            total += 10;
        } else {  // 2-10
            total += value;
        }
    }

    // Convert Aces from 11 to 1 if needed
    while (total > 21 && aces > 0) {
        total -= 10;
        aces--;
    }

    return total;
}

void show_cards(int* cards, int count, char* owner) {
    print_string(owner);
    print_string(": ");

    for (int i = 0; i < count; i++) {
        int value = get_card_value(cards[i]);
        int suit = get_card_suit(cards[i]);

        print_card_value(value);
        print_suit(suit);

        if (i < count - 1) {
            print_string(", ");
        }
    }

    print_string(" (");
    int total = calculate_hand(cards, count);
    if (total >= 10) {
        print_char('0' + (total / 10));
        print_char('0' + (total % 10));
    } else {
        print_char('0' + total);
    }
    print_string(")\n\r");
}

void kernel_main() {
    clear_screen();
    print_string("=== BLACKJACK 21 ===\n\r");
    print_string("Press any key to start...\n\r");
    char first_key = read_key();

    // Seed random with keypress
    seed = (unsigned int)first_key * 31 + 17;

    while (1) {
        player_count = 0;
        dealer_count = 0;

        clear_screen();
        print_string("=== NEW ROUND ===\n\r");

        // Deal initial cards
        player_cards[player_count++] = give_card();
        dealer_cards[dealer_count++] = give_card();
        player_cards[player_count++] = give_card();
        dealer_cards[dealer_count++] = give_card();

        // Player turn
        while (1) {
            print_string("\n\r");
            show_cards(player_cards, player_count, "Player");
            print_string("Dealer: ");

            int value = get_card_value(dealer_cards[0]);
            int suit = get_card_suit(dealer_cards[0]);
            print_card_value(value);
            print_suit(suit);
            print_string(", ?\n\r");

            int player_total = calculate_hand(player_cards, player_count);

            if (player_total > 21) {
                print_string("\n\rBust! You lose!\n\r");
                break;
            }

            print_string("\n\r(H)it or (S)tand? ");
            char choice = read_key();
            print_char(choice);

            if (choice == 'h' || choice == 'H') {
                player_cards[player_count++] = give_card();
            } else {
                break;
            }
        }

        // Dealer turn
        int player_total = calculate_hand(player_cards, player_count);
        if (player_total <= 21) {
            print_string("\n\r\n\rDealer's turn...\n\r");

            while (calculate_hand(dealer_cards, dealer_count) < 17) {
                dealer_cards[dealer_count++] = give_card();
            }

            show_cards(dealer_cards, dealer_count, "Dealer");

            int dealer_total = calculate_hand(dealer_cards, dealer_count);

            if (dealer_total > 21) {
                print_string("Dealer bust! You win!\n\r");
            } else if (dealer_total > player_total) {
                print_string("Dealer wins!\n\r");
            } else if (player_total > dealer_total) {
                print_string("You win!\n\r");
            } else {
                print_string("Tie!\n\r");
            }
        }

        print_string("\n\rPlay again? (Y/N) ");
        char again = read_key();
        print_char(again);

        if (again != 'y' && again != 'Y') {
            break;
        }
    }

    print_string("\n\rThanks for playing!\n\r");
    while (1);
}