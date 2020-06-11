#ifndef HAND_H
#define HAND_H

#include "card.h"
#include <string.h>
#include <stdio.h>

typedef struct hand_struct {

    card_t cards[5];
    int type;
    int total_value;
    char high_card_rank[3];
    int high_card_value;

} hand_t;

typedef enum {
    Nothing,
    OnePair,
    TwoPairs,
    ThreeKind,
    Straight,
    Flush,
    FullHouse,
    FourKind,
    StraightFlush,
    RoyalFlush
} hand_ranking_t;

void evaluateHand(hand_t * hand);
int hasFourOfaKind(hand_t * hand);
int isFullHouse(hand_t * hand);
int isFlush(hand_t * hand);
int isStraight(hand_t * hand);
int hasThreeOfaKind(hand_t * hand);
int hasTwoPairs(hand_t * hand);
int hasOnePair(hand_t * hand);
void sortCardsByRank(hand_t * hand);
void sortCardsBySuit(card_t cards[5]);
void printHand(hand_t hand);
hand_t compareHands(hand_t hand, hand_t other_hand);
void changeAllCards(hand_t * table_hand, hand_t * player_hand);
void changeOneCard(hand_t * table_hand, hand_t * player_hand);
int firstHandIsHigher(hand_t hand, hand_t other_hand);

#endif