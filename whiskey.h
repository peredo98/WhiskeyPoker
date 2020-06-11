
#ifndef WHISKEY_H
#define WHISKEY_H

#include "hand.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "player.h"
#include "codes.h"

// For srand function
#include <time.h>

#define MAX_PLAYERS 8
#define LIVES 2

typedef struct whiskey_struct { //Main structure for status of the game

    int gameBet;
    int betAgreement; // Tells if all the players agreed on the bet 
    int lowestAmount; // The lowest amount given by the players
    int prize; //The amount to give to the winner
	hand_t table_hand;
    int numPlayers; // Number of players to start playing the game
    player_t * players_array;
	int playerInTurn; //id of player in turn
    int index_playerInTurn; //id of player in turn
    int winner;
    int playersReady;
    int gameStatus;
    int index_startRoundIndex;
} whiskey_t;

void dealCards(whiskey_t * whiskey_data);
void chooseWhiskey(whiskey_t *whiskey_data);
void addNewPlayer(int playerId, whiskey_t * whiskey_data);
void removePlayer(int playerId, whiskey_t * whiskey_data);
void assignTurns(player_t * players_array, int numPlayers);
void initGame(whiskey_t * whiskey_data);

#endif