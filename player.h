#ifndef PLAYER_H
#define PLAYER_H

#include "hand.h"


// Data for a single player
typedef struct player_struct {
    int id;
    int connected;
    int lost;
    int status;
    int bet;
    hand_t hand;
    // card_t cards[5];
    // int hand_value;
    // char high_card_rank[3];
    // int high_card_value;
} player_t;

#endif