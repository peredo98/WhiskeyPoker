/*
    Emiliano Peredo A01422326
    Raziel Nicolás Martínez Castillo A01410695
    Camila Rovirosa Ochoa A01024192

    Proyect: Whiskey Poker
    ♥♦♣♠
*/
#include "whiskey.h"

void dealCards(whiskey_t * whiskey_data){

    const char cardsArray[13][3]= {"A","2","3","4","5","6","7","8","9","10","J","Q","K"};
    const int valuesArray[13]= {14,2,3,4,5,6,7,8,9,10,11,12,13};
    const char suitsArray[4][9]= {"♥","♦","♣","♠"}; 
    const int suitsValueArray[4]= {1, 2, 3,4};
    card_t cards[52];
    card_t random_card;
    int random_number;
    int counter = 0;
    int given_cards;
    srand(time(NULL));

    for (int i = 0; i < 13; i++) {
        for(int j = 0; j < 4; j++){
            strcpy(cards[counter].rank, cardsArray[i]);
            strcpy(cards[counter].suit, suitsArray[j]);
            cards[counter].rank_value = valuesArray[i];
            cards[counter].suit_value = suitsValueArray[j];
            cards[counter].used = 0;
            counter++;
        }
    }

    //Print cards
    // for(int i = 0; i < 52; i++) {
    //     printf("#%d, Card %s, Value %d, Suit %s, SuitValue %d Used? %d\n", i+1, cards[i].rank, cards[i].rank_value, cards[i].suit, cards[i].suit_value, cards[i].used);
    // }

    //generate players cards
    for(int i = 0; i<whiskey_data->numPlayers; i++){
        given_cards = 0;
        while(given_cards < 5){
            random_number = rand() % 52;
            random_card = cards[random_number];
            if(random_card.used){
                continue;
            } else {
                strcpy(whiskey_data->players_array[i].hand.cards[given_cards].rank, random_card.rank);
                strcpy(whiskey_data->players_array[i].hand.cards[given_cards].suit, random_card.suit);
                whiskey_data->players_array[i].hand.cards[given_cards].rank_value = cards[random_number].rank_value;
                whiskey_data->players_array[i].hand.cards[given_cards].suit_value = cards[random_number].suit_value;
                cards[random_number].used = 1;
                given_cards++;
            }
        }
    }

    //generate table cards
    given_cards = 0;
    while(given_cards < 5){
        //generate table cards
        random_number = rand() % 52;
        random_card = cards[random_number];
        if(random_card.used){
            continue;
        } else {
            strcpy(whiskey_data->table_hand.cards[given_cards].rank, random_card.rank);
            strcpy(whiskey_data->table_hand.cards[given_cards].suit, random_card.suit);
            whiskey_data->table_hand.cards[given_cards].rank_value = random_card.rank_value;
            whiskey_data->table_hand.cards[given_cards].suit_value = random_card.suit_value;
            cards[random_number].used = 1;
            given_cards++;
        }
    }

    //print player cards
    // for(int i = 0; i<((whiskey_data->numPlayers) + 1); i++){
    //     if(i == whiskey_data->numPlayers){
    //         printf("Table cards:\n");
    //         for(int j = 0; j<5; j++){
    //             printf("Cart: %s, Suit %s, SuitValue: %d, Value %d\n", whiskey_data->table_hand.cards[j].rank, whiskey_data->table_hand.cards[j].suit, whiskey_data->table_hand.cards[j].suit_value, whiskey_data->table_hand.cards[j].rank_value);
    //         }
    //         printf("\n");
    //     }else {
    //         printf("Player: %d\n", whiskey_data->players_array[i].id);
    //         for(int j = 0; j<5; j++){
    //             printf("Cart: %s, Suit %s, SuitValue: %d, Value %d\n", whiskey_data->players_array[i].hand.cards[j].rank, whiskey_data->players_array[i].hand.cards[j].suit, whiskey_data->players_array[i].hand.cards[j].suit_value, whiskey_data->players_array[i].hand.cards[j].rank_value);
    //         }
    //         printf("\n");
    //     }
    // }

    //Print cards
    // for(int i = 0; i < 52; i++) {
    //     printf("#%d, Card %s, Suit %s, SuitValue: %d, Used? %d, Value %d\n", i+1, cards[i].rank, cards[i].suit, cards[i].suit_value, cards[i].used, cards[i].rank_value);
    // }

    //sort player cards
    for(int i = 0; i<((whiskey_data->numPlayers) + 1); i++){
        if(i == whiskey_data->numPlayers){
            sortCardsByRank(&whiskey_data->table_hand);
        }else {
            sortCardsByRank(&whiskey_data->players_array[i].hand);
        }
    }

    //print player cards
    /*for(int i = 0; i<((whiskey_data->numPlayers) + 1); i++){
        if(i == whiskey_data->numPlayers){
            printf("Sorted Table cards:\n");
            for(int j = 0; j<5; j++){
                printf("Cart: %s, Suit %s, SuitValue %d,Value %d\n", whiskey_data->table_hand.cards[j].rank, whiskey_data->table_hand.cards[j].suit, whiskey_data->table_hand.cards[j].suit_value, whiskey_data->table_hand.cards[j].rank_value);
            }
            printf("\n");
        }else {
            printf("Sorted Player %d Cards\n", whiskey_data->players_array[i].id);
            for(int j = 0; j<5; j++){
                printf("Cart: %s, Suit %s, SuitValue %d, Value %d\n", whiskey_data->players_array[i].hand.cards[j].rank, whiskey_data->players_array[i].hand.cards[j].suit, whiskey_data->players_array[i].hand.cards[j].suit_value, whiskey_data->players_array[i].hand.cards[j].rank_value);
            }
            printf("\n");
        }
    }*/

    /*printf("\n\nTESTING EVALUATION \n\n");

    for(int i = 0; i<((whiskey_data->numPlayers) + 1); i++){
        if(i == whiskey_data->numPlayers){
            
            printf("Evaluated table\n");
            for(int j = 0; j<5; j++){
                printf("Cart: %s, Suit %s, SuitValue %d,Value %d\n", whiskey_data->table_hand.cards[j].rank, whiskey_data->table_hand.cards[j].suit, whiskey_data->table_hand.cards[j].suit_value, whiskey_data->table_hand.cards[j].rank_value);
            }
            evaluateHand(&whiskey_data->table_hand);
            printf("Type: %d, Untie Value: %d, HighCardRank: %s,  HighCardValue: %d\n", whiskey_data->table_hand.type, whiskey_data->table_hand.total_value, whiskey_data->table_hand.high_card_rank, whiskey_data->table_hand.high_card_value);
        }else {
            
            printf("Evaluated player %d \n", whiskey_data->players_array[i].id);
            for(int j = 0; j<5; j++){
                printf("Cart: %s, Suit %s, SuitValue %d, Value %d\n", whiskey_data->players_array[i].hand.cards[j].rank, whiskey_data->players_array[i].hand.cards[j].suit, whiskey_data->players_array[i].hand.cards[j].suit_value, whiskey_data->players_array[i].hand.cards[j].rank_value);
            }
            evaluateHand(&whiskey_data->players_array[i].hand);
            printf("Type: %d, Untie Value: %d, HighCardRank: %s,  HighCardValue: %d\n", whiskey_data->players_array[i].hand.type, whiskey_data->players_array[i].hand.total_value, whiskey_data->players_array[i].hand.high_card_rank,whiskey_data->players_array[i].hand.high_card_value);
        }
    }*/
}


void addNewPlayer(int playerId, whiskey_t * whiskey_data) {

    for(int i = 0; i<MAX_PLAYERS; i++){
        if(whiskey_data->players_array[i].id == 0){
            whiskey_data->players_array[i].id = playerId;
            whiskey_data->players_array[i].connected = 1;
            whiskey_data->players_array[i].lost = 0;
            whiskey_data->players_array[i].lives = LIVES;
            whiskey_data->players_array[i].status = LOBBY;
            printf("Added player with ID: %d\n", whiskey_data->players_array[i].id);
            break;
        }
    }

}

void removePlayer(int playerId, whiskey_t * whiskey_data) {

    for(int i = 0; i<MAX_PLAYERS; i++){
        if(whiskey_data->players_array[i].id == playerId){
            whiskey_data->players_array[i].id = 0;
            whiskey_data->players_array[i].connected = 0;
            whiskey_data->numPlayers--;
            printf("Removed player with ID: %d, Number of platers: %d\n", whiskey_data->players_array[i].id, whiskey_data->numPlayers);
            break;
        }
    }
}

void initGame(whiskey_t * whiskey_data) {

    whiskey_data->numPlayers = 0;
    whiskey_data->gameBet = 0;
    whiskey_data->prize = 0;
    whiskey_data->winner = 0;
    whiskey_data->playerInTurn = 0;
    whiskey_data->index_playerInTurn = 0;
    whiskey_data->playersReady =  0;
    whiskey_data->lowestAmount =  0;
    whiskey_data->gameStatus = WAIT;

    whiskey_data->players_array = malloc(MAX_PLAYERS * sizeof (player_t));

    for(int i = 0; i<MAX_PLAYERS; i++){
        whiskey_data->players_array[i].id = 0;
        whiskey_data->players_array[i].status = WAIT;
    }
}

void chooseWhiskey (whiskey_t *whiskey_data){
    
    int option;
    int chooseCard;
    int chooseTableCard;
    //full hand
    hand_t playerHand = whiskey_data->players_array[0].hand;
    hand_t tableHand = whiskey_data->table_hand;
    hand_t temp;

    //one card
    card_t tempCard;

    while(1){ //Keep asking for a valid bet
            printf("Si quieres la viuda entera presiona 1, si quieres solo una carta presiona 2\n");
            scanf("%d", &option);
            if(option == 1){
                printf("Toma la viuda entera");
                temp = playerHand;
                playerHand = tableHand;
                tableHand = temp;
                whiskey_data->table_hand = tableHand;
                whiskey_data->players_array[0].hand=playerHand;
                break;
            } 
            else if(option == 2) {
                while(1){
                    printf("Escoge la carta que vas a cambiar: ");
                    printf("Card 1 = %s %s\n",playerHand.cards[0].rank, playerHand.cards[0].suit);
                    printf("Card 2 = %s %s\n",playerHand.cards[1].rank, playerHand.cards[1].suit);
                    printf("Card 3 = %s %s\n",playerHand.cards[2].rank, playerHand.cards[2].suit);
                    printf("Card 4 = %s %s\n",playerHand.cards[3].rank, playerHand.cards[3].suit);
                    printf("Card 5 = %s %s\n",playerHand.cards[4].rank, playerHand.cards[4].suit);
                    scanf("%d", &chooseCard);

                    printf("Escoge una sola carta: ");
                    printf("Card 1 = %s %s\n",tableHand.cards[0].rank, tableHand.cards[0].suit);
                    printf("Card 2 = %s %s\n",tableHand.cards[1].rank, tableHand.cards[1].suit);
                    printf("Card 3 = %s %s\n",tableHand.cards[2].rank, tableHand.cards[2].suit);
                    printf("Card 4 = %s %s\n",tableHand.cards[3].rank, tableHand.cards[3].suit);
                    printf("Card 5 = %s %s\n",tableHand.cards[4].rank, tableHand.cards[4].suit);
                    scanf("%d", &chooseTableCard);
                    
                    if(chooseCard<1 || chooseCard>5 || chooseTableCard<1 || chooseTableCard>5){
                        printf("Error choose a number between 1 and 5 \n");
                    }
                    else{
                        tempCard = tableHand.cards[chooseTableCard-1];
                        tableHand.cards[chooseTableCard-1] =playerHand.cards[chooseCard-1];
                        playerHand.cards[chooseCard-1] = tempCard; 
                        break;
                    }
                }
                evaluateHand(&playerHand);
                sortCardsByRank(&playerHand);
                evaluateHand(&tableHand);
                sortCardsByRank(&tableHand);
                whiskey_data->table_hand = tableHand;
                whiskey_data->players_array[0].hand=playerHand;
                break;
            }
            else {
               printf("Opcion no valida, vuelve a intentar");
            }
    }
    for(int i = 0; i<((whiskey_data->numPlayers) + 1); i++){
        if(i == whiskey_data->numPlayers){
            printf("Changed Table cards:\n");
            for(int j = 0; j<5; j++){
                printf("Cart: %s, Suit %s, SuitValue %d,Value %d\n", whiskey_data->table_hand.cards[j].rank, whiskey_data->table_hand.cards[j].suit, whiskey_data->table_hand.cards[j].suit_value, whiskey_data->table_hand.cards[j].rank_value);
            }
            printf("\n");
        }else {
            printf("Changed Player %d Cards\n", whiskey_data->players_array[i].id);
            for(int j = 0; j<5; j++){
                printf("Cart: %s, Suit %s, SuitValue %d, Value %d\n", whiskey_data->players_array[i].hand.cards[j].rank, whiskey_data->players_array[i].hand.cards[j].suit, whiskey_data->players_array[i].hand.cards[j].suit_value, whiskey_data->players_array[i].hand.cards[j].rank_value);
            }
            printf("\n");
        }
    }
}

void assignTurns(player_t * players_array, int numPlayers){

    player_t * tempArray;
    int index = 0;

    tempArray = malloc(numPlayers * sizeof (player_t));

    for(int i = 0; i < MAX_PLAYERS; i++){
        if(players_array[i].id > 0){
            tempArray[index] = players_array[i];
            index++;
        }
    }

    for(int i = 0; i < MAX_PLAYERS; i++){
        if(i<numPlayers){
            players_array[i] = tempArray[i];
        } else {
            players_array[i].id = 0;
            players_array[i].connected = 0;
        }
    }

}

void winner (whiskey_t *whiskey_data, int numPlayers){
    int tempWinType = -1;
    int playerWinnerId;
    player_t * tempArr = malloc(numPlayers * sizeof (player_t));;
    for(int i = 0; i<((whiskey_data->numPlayers) + 1); i++){
        
        if(whiskey_data->players_array[i].hand.type < tempWinType){
            printf("Impossible No Winners \n");
        }
        else{
             tempArr[1] = whiskey_data->players_array[i];
             if(whiskey_data->players_array[i].hand.type > tempArr[1].hand.type){
                 playerWinnerId =whiskey_data->players_array[i].id;
             }
             else if (whiskey_data->players_array[i].hand.type = tempArr[1].hand.type){
                 if(whiskey_data->players_array[i].hand.total_value > tempArr[1].hand.total_value){
                     playerWinnerId =whiskey_data->players_array[i].id;
                 }else{
                     playerWinnerId = tempArr[1].id;
                 }
             }else{
                 playerWinnerId = tempArr[1].id;
             }
        }
      
    }
        printf("id: %d \n",playerWinnerId);
     for(int i = 0; i<((whiskey_data->numPlayers) + 1); i++){
         if(whiskey_data->players_array[i].id = playerWinnerId){
             printf("Winner Player");
             for(int j = 0; j<5; j++){
                printf("Cart: %s, Suit %s, SuitValue %d, Value %d\n", whiskey_data->players_array[i].hand.cards[j].rank, whiskey_data->players_array[i].hand.cards[j].suit, whiskey_data->players_array[i].hand.cards[j].suit_value, whiskey_data->players_array[i].hand.cards[j].rank_value);
            }
         }
     }
}