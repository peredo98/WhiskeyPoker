/*
    Emiliano Peredo A01422326
    Raziel Nicolás Martínez Castillo A01410695
    Camila Rovirosa Ochoa A01024192

    Proyect: Whiskey Poker

*/
#ifndef PLAYER_H
#define PLAYER_H

#include "hand.h"


// Data for a single player
typedef struct player_struct {
    int id;
    int connected;
    int lost;
    int status;
    int lives;
    int bet;
    hand_t hand;

} player_t;

#endif