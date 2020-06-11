
#include "hand.h"

#define MAXCARDS 21 //A player can get at most 21 cards (21 A's)
#define MAXLENGTH 3 // Needed to save strings in the array, '10' having the most characters


typedef enum {WAIT, LOBBY, READY} status_t;

// Define constants for the messages in the protocol
typedef enum {PLAY, START, AMOUNT, BET, BYE, WAITING, TURN, KNOCK, CHANGE_ONE, CHANGE_ALL, PASS, FULL} code_t;

// Structure to be sent between client and server
typedef struct {
    code_t msg_code;
    status_t theStatus;
    code_t playerStatus;
    code_t dealerStatus;
    int playerAmount;
    int playerBet;
    int numPlayerCards;
    int lowestAmount;
    int turn;
    int playerLives;
    hand_t playerHand;
    hand_t whiskeyHand;
} message_t;
