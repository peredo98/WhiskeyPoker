typedef struct card_struct { //Main structure for status of the game
    char rank[3];
    int rank_value;
    char suit[9];
    int suit_value;
    int used;
} card_t;