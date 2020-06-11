/*
    Emiliano Peredo A01422326
    Raziel Nicolás Martínez Castillo A01410695
    Camila Rovirosa Ochoa A01024192

    Proyect: Whiskey Poker

*/
#include "hand.h"

void evaluateHand(hand_t * hand){

    if((isFlush(hand) == 1) && (isStraight(hand) == 2)){
        hand->type = RoyalFlush;
    } else if((isFlush(hand) == 1) && (isStraight(hand) == 1)){
        hand->type = StraightFlush;
    } else if(hasFourOfaKind(hand) == 1){
        hand->type = FourKind;
    } else if (isFullHouse(hand) == 1){
        hand->type = FullHouse;
    } else if (isFlush(hand) == 1){
        hand->type = Flush;
    } else if ((isStraight(hand) == 1) || (isStraight(hand) == 2)){
        hand->type = Straight;
    } else if (hasThreeOfaKind(hand) == 1){
        hand->type = ThreeKind;
    } else if (hasTwoPairs(hand) == 1){
        hand->type = TwoPairs;
    } else if (hasOnePair(hand) == 1){
        hand->type = OnePair;
    } else {
        hand->high_card_value = hand->cards[4].rank_value;
        strcpy(hand->high_card_rank, hand->cards[4].rank);
        hand->type = Nothing;
    }
}

int hasFourOfaKind(hand_t * hand){

    if(hand->cards[0].rank_value  == hand->cards[3].rank_value)
    {      
        hand->total_value = hand->cards[0].rank_value * 4;
        hand->high_card_value = hand->cards[4].rank_value;
        strcpy(hand->high_card_rank, hand->cards[4].rank);
        return 1;
    }
    else if (hand->cards[1].rank_value == hand->cards[4].rank_value)
    {
        hand->total_value = hand->cards[1].rank_value * 4;
        hand->high_card_value = hand->cards[0].rank_value;
        strcpy(hand->high_card_rank, hand->cards[0].rank);
        return 1;
    }

    return 0;
}

int isFullHouse(hand_t * hand){
    //xxxaa or aaxxx
    if ((hand->cards[0].rank_value == hand->cards[1].rank_value && hand->cards[0].rank_value == hand->cards[2].rank_value && hand->cards[3].rank_value == hand->cards[4].rank_value) ||
        (hand->cards[0].rank_value == hand->cards[1].rank_value && hand->cards[2].rank_value == hand->cards[3].rank_value && hand->cards[2].rank_value == hand->cards[4].rank_value))
    {
        hand->total_value = (hand->cards[0].rank_value) + (hand->cards[1].rank_value) + (hand->cards[2].rank_value) +
            (hand->cards[3].rank_value) + (hand->cards[4].rank_value);
        return 1;
    }

    return 0;
}

int isFlush(hand_t * hand){// All cards has same suit

    card_t temp[5];

    for(int j = 0; j<5; j++){
        temp[j] = hand->cards[j];
    }

    sortCardsBySuit(temp);      // Sort the cards by the suit values

   if(temp[0].suit_value == temp[4].suit_value ){
       hand->total_value = temp[4].rank_value;
       return 1;
   }else {
       return 0;
   }
}

int isStraight(hand_t * hand){ 
        //if 5 consecutive values
        if ( hand->cards[4].rank_value == 14 ) //If the hand contains an A
        {
            if(hand->cards[0].rank_value == 2 && hand->cards[1].rank_value == 3 &&
                hand->cards[2].rank_value == 4 && hand->cards[3].rank_value == 5){
                    return 1;
            }else if (hand->cards[0].rank_value == 10 && hand->cards[1].rank_value == 11 &&        
                hand->cards[2].rank_value == 12 && hand->cards[3].rank_value == 13){
                return 2;
            } else {
                return 0;
            }
        } else {
            if(hand->cards[0].rank_value + 1 == hand->cards[1].rank_value  &&
            hand->cards[1].rank_value  + 1 == hand->cards[2].rank_value  &&
            hand->cards[2].rank_value  + 1 == hand->cards[3].rank_value  &&
            hand->cards[3].rank_value  + 1 == hand->cards[4].rank_value )
            {
                //player with the highest value of the last card wins
                hand->total_value = hand->cards[4].rank_value ;
                return 1;
            }else{
                return 0;
            }
        }        
}

int hasThreeOfaKind(hand_t * hand){
    //if the 1,2,3 cards are the same OR xxxab
    //2,3,4 cards are the same OR  axxxb
    //3,4,5 cards are the same  abxxx
    //3rds card will always be a part of Three of A Kind
    if ((hand->cards[0].rank_value == hand->cards[1].rank_value && hand->cards[0].rank_value == hand->cards[2].rank_value) ||
    (hand->cards[1].rank_value == hand->cards[2].rank_value && hand->cards[1].rank_value == hand->cards[3].rank_value))
    {
        hand->total_value = hand->cards[2].rank_value * 3;
        hand->high_card_value = hand->cards[4].rank_value;
        strcpy(hand->high_card_rank, hand->cards[4].rank);
        return 1;
    }
    else if (hand->cards[2].rank_value == hand->cards[3].rank_value && hand->cards[2].rank_value == hand->cards[4].rank_value)
    {
        hand->total_value = hand->cards[2].rank_value * 3;
        hand->high_card_value = hand->cards[1].rank_value;
        strcpy(hand->high_card_rank, hand->cards[1].rank);
        return 1;
    }
    return 0;
}

int hasTwoPairs(hand_t * hand){
    //if 1,2 and 3,4 aabbx
    //if 1.2 and 4,5 aaxbb
    //if 2.3 and 4,5 xaabb
    //with two pairs, the 2nd card will always be a part of one pair 
    //and 4th card will always be a part of second pair
    if (hand->cards[0].rank_value == hand->cards[1].rank_value && hand->cards[2].rank_value == hand->cards[3].rank_value)
    {
        hand->total_value = (hand->cards[1].rank_value * 2) + (hand->cards[3].rank_value * 2);
        hand->high_card_value = hand->cards[4].rank_value;
        strcpy(hand->high_card_rank, hand->cards[4].rank);
        return 1;
    }
    else if (hand->cards[0].rank_value == hand->cards[1].rank_value && hand->cards[3].rank_value == hand->cards[4].rank_value)
    {
        hand->total_value = (hand->cards[1].rank_value * 2) + (hand->cards[3].rank_value * 2);
        hand->high_card_value = hand->cards[2].rank_value;
        strcpy(hand->high_card_rank, hand->cards[2].rank);
        return 1;
    }
    else if (hand->cards[1].rank_value == hand->cards[2].rank_value && hand->cards[3].rank_value == hand->cards[4].rank_value)
    {
        hand->total_value = (hand->cards[1].rank_value * 2) + (hand->cards[3].rank_value * 2);
        hand->high_card_value = hand->cards[0].rank_value;
        strcpy(hand->high_card_rank, hand->cards[0].rank);
        return 1;
    }
    return 0;
}

int hasOnePair(hand_t * hand){
    //if 1,2 -> 5th card has the highest value
    //2.3
    //3,4
    //4,5 -> card #3 has the highest value
    if (hand->cards[0].rank_value == hand->cards[1].rank_value)
    {
        hand->total_value = hand->cards[0].rank_value * 2;
        hand->high_card_value = hand->cards[4].rank_value;
        strcpy(hand->high_card_rank, hand->cards[4].rank);
        return 1;
    }
    else if (hand->cards[1].rank_value == hand->cards[2].rank_value)
    {
        hand->total_value = hand->cards[1].rank_value * 2;
        hand->high_card_value = hand->cards[4].rank_value;
        strcpy(hand->high_card_rank, hand->cards[4].rank);
        return 1;
    }
    else if (hand->cards[2].rank_value == hand->cards[3].rank_value)
    {
        hand->total_value = hand->cards[2].rank_value * 2;
        hand->high_card_value = hand->cards[4].rank_value;
        strcpy(hand->high_card_rank, hand->cards[4].rank);
        return 1;
    }
    else if (hand->cards[3].rank_value == hand->cards[4].rank_value)
    {
        hand->total_value = hand->cards[3].rank_value * 2;
        hand->high_card_value = hand->cards[2].rank_value;
        strcpy(hand->high_card_rank, hand->cards[2].rank);
        return 1;
    }else {
        return 0;
    }
}


void sortCardsByRank(hand_t * hand){

    int min;
    card_t temp;

    for(int i = 0; i<5; i++){
        min = i;
        for(int j = i+1; j<5; j++){
            if(hand->cards[min].rank_value>hand->cards[j].rank_value){
                min = j;
            }
        }
        temp = hand->cards[i];
        hand->cards[i] = hand->cards[min];
        hand->cards[min] = temp;
    }

}

void sortCardsBySuit(card_t cards[5]){

    int min;
    card_t temp;

    for(int i = 0; i<5; i++){
        min = i;
        for(int j = i+1; j<5; j++){
            if(cards[min].suit_value>cards[j].suit_value){
                min = j;
            }
        }
        temp = cards[i];
        cards[i] = cards[min];
        cards[min] = temp;
    }
}

void printHand(hand_t hand){
    for(int j = 0; j<5; j++){
        printf("Card: %s, Suit %s, SuitValue %d,Value %d\n", hand.cards[j].rank, hand.cards[j].suit, hand.cards[j].suit_value, hand.cards[j].rank_value);
    }
    evaluateHand(&hand);
    printf("Type: %d, Untie Value: %d, HighCardRank: %s,  HighCardValue: %d\n", hand.type, hand.total_value, hand.high_card_rank, hand.high_card_value);
}

hand_t compareHands(hand_t hand, hand_t other_hand){

    if(hand.type > other_hand.type){
        printf("The first hand provided is better.\n");
        return hand;
    }else if(hand.type < other_hand.type){
        printf("The second hand provided is better.\n");
        return other_hand;
    } else { //If the hand types are equal
        if(hand.total_value > other_hand.total_value){
             printf("Both hands are of the same type but the first player has a higher total value.\n");
             return hand;
        }else if(hand.total_value < other_hand.total_value){
             printf("Both hands are of the same type but the second player has a higher total value.\n");
             return other_hand;
        } else {        
            printf("This is a draw, no one wins.\n");
            return hand;
        }
    }

}

int firstHandIsHigher(hand_t hand, hand_t other_hand){

    if(hand.type > other_hand.type){
        printf("The first hand provided is better.\n");
        return 1;
    }else if(hand.type < other_hand.type){
        printf("The second hand provided is better.\n");
        return 0;
    } else { //If the hand types are equal
        if(hand.total_value > other_hand.total_value){
             printf("Both hands are of the same type but the first player has a higher total value.\n");
             return 1;
        }else if(hand.total_value < other_hand.total_value){
             printf("Both hands are of the same type but the second player has a higher total value.\n");
             return 0;
        } else {        
            printf("This is a draw, no one wins.\n");
            return 1;
        }
    }
}

void changeAllCards(hand_t * table_hand, hand_t * player_hand){

    hand_t temp;

    temp = *player_hand;
    *player_hand = *table_hand;
    *table_hand = temp;

    evaluateHand(player_hand);
    sortCardsByRank(player_hand);
    evaluateHand(table_hand);
    sortCardsByRank(table_hand);
}

void changeOneCard(hand_t * table_hand, hand_t * player_hand){

    int chooseCard;
    card_t tempCard;
    int chooseTableCard;

    while(1){
        printf("Escoge la carta que vas a cambiar: \n");
        printf("Card 1 = %s %s\n",player_hand->cards[0].rank, player_hand->cards[0].suit);
        printf("Card 2 = %s %s\n",player_hand->cards[1].rank, player_hand->cards[1].suit);
        printf("Card 3 = %s %s\n",player_hand->cards[2].rank, player_hand->cards[2].suit);
        printf("Card 4 = %s %s\n",player_hand->cards[3].rank, player_hand->cards[3].suit);
        printf("Card 5 = %s %s\n",player_hand->cards[4].rank, player_hand->cards[4].suit);
        scanf("%d", &chooseCard);

        printf("Escoge una sola carta: \n");
        printf("Card 1 = %s %s\n",table_hand->cards[0].rank, table_hand->cards[0].suit);
        printf("Card 2 = %s %s\n",table_hand->cards[1].rank, table_hand->cards[1].suit);
        printf("Card 3 = %s %s\n",table_hand->cards[2].rank, table_hand->cards[2].suit);
        printf("Card 4 = %s %s\n",table_hand->cards[3].rank, table_hand->cards[3].suit);
        printf("Card 5 = %s %s\n",table_hand->cards[4].rank, table_hand->cards[4].suit);
        scanf("%d", &chooseTableCard);
        
        if(chooseCard<1 || chooseCard>5 || chooseTableCard<1 || chooseTableCard>5){
            printf("Error choose a number between 1 and 5 \n");
        }
        else{
            tempCard = table_hand->cards[chooseTableCard-1];
            table_hand->cards[chooseTableCard-1] =player_hand->cards[chooseCard-1];
            player_hand->cards[chooseCard-1] = tempCard; 
            break;
        }
    }

    evaluateHand(player_hand);
    sortCardsByRank(player_hand);
    evaluateHand(table_hand);
    sortCardsByRank(table_hand);

}