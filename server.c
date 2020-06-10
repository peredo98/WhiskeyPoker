/*
    Program for a simple bank server
    It uses sockets and threads
    The server will process simple transactions on a limited number of accounts

    Raziel Nicolás Martínez Castillo A01410695
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Signals library
#include <errno.h>
#include <signal.h>
// Sockets libraries
#include <netdb.h>
#include <sys/poll.h>
// Posix threads library
#include <pthread.h>
// For srand function
#include <time.h>

// Custom libraries
#include "codes.h"
#include "sockets.h"

#define MAX_ACCOUNTS 5
#define BUFFER_SIZE 1024
#define MAX_QUEUE 5
#define MAX_PLAYERS 8

///// Structure definitions

// Data for a single bank account
typedef struct account_struct {
    int id;
    int pin;
    float balance;
} account_t;

// Data for the bank operations
typedef struct bank_struct {
    // Store the total number of operations performed
    int total_transactions;
    // An array of the accounts
    account_t * account_array;
} bank_t;

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



// // Data that will be sent to each thread
// typedef struct data_struct {
//     // The file descriptor for the socket
//     int connection_fd;
//     // A pointer to a bank data structure
//     bank_t * bank_data;
//     // A pointer to a locks structure
//     locks_t * data_locks;
// } thread_data_t;

typedef struct card_struct { //Main structure for status of the game
    char rank[3];
    int rank_value;
    char suit[9];
    int suit_value;
    int used;
} card_t;

typedef struct hand_struct {

    card_t cards[5];
    int type;
    int total_value;
    char high_card_rank[3];
    int high_card_value;

} hand_t;

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

typedef struct viuda_struct { //Main structure for status of the game

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

} viuda_t;

// Data that will be sent to each thread
typedef struct data_struct {
    // The file descriptor for the socket
    message_t message;
    int connection_fd;
    int playerId;
    // int agreeBet;
    // // A pointer to a bank data structure
    // bank_t * bank_data;
    viuda_t * viuda_data;
    // // A pointer to a locks structure
    // locks_t * data_locks;
} thread_data_t;


// Global variables for signal handlers
int interrupt_exit = 0;
pthread_cond_t start_condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t ready_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t getBets_condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t bets_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t betsReady_condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t betsReady_mutex = PTHREAD_MUTEX_INITIALIZER;

///// FUNCTION DECLARATIONS
void usage(char * program);
void setupHandlers();
void detectInterruption(int signal);
// void initBank(bank_t * bank_data, locks_t * data_locks);
// void readBankFile(bank_t * bank_data);
// void waitForConnections(int server_fd, bank_t * bank_data, locks_t * data_locks);
void initGame(viuda_t * viuda_data);
void waitForConnections(int server_fd, viuda_t * viuda_data);
void * attentionThread(void * arg);
void addNewPlayer(int playerId, viuda_t * viuda_data);
// void closeBank(bank_t * bank_data, locks_t * data_locks);
int checkValidAccount(int account);
void storeChanges(bank_t * bank_data);
void completeFirstDeal(message_t * message, int connection_fd);
void assignTurns(player_t * players_array, int numPlayers);
void dealCards(viuda_t * viuda_data);
void sortCardsByRank(hand_t * hand);
void sortCardsBySuit(card_t cards[5]);
void evaluateHand(hand_t * hand);
int hasFourOfaKind(hand_t * hand);
int isFullHouse(hand_t * hand);
int isFlush(hand_t * hand);
int isStraight(hand_t * hand);
int hasThreeOfaKind(hand_t * hand);
int hasTwoPairs(hand_t * hand);
int hasOnePair(hand_t * hand);
void chooseViuda(viuda_t *viuda_data);
hand_t compareHands(hand_t hand, hand_t other_hand);

/*
    TODO: Add your function declarations here
*/
void calculateResults(message_t * message);
void dealerTurn(message_t * message, int connection_fd);
void playerTurn(message_t * message, int connection_fd);
int getRandomCard(message_t * message, char pord);

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int server_fd;
    // bank_t bank_data;
    // locks_t data_locks;

    viuda_t viuda_data;

    printf("\n=== SERVER PROGRAM ===\n");

    // Check the correct arguments
    if (argc != 2)
    {
        usage(argv[0]);
    }

    // Configure the handler to catch SIGINT
    // setupHandlers();

    // Initialize the data structures
    // initBank(&bank_data, &data_locks);
    initGame(&viuda_data);

	// Show the IPs assigned to this computer
	printLocalIPs();
    // Start the server
    server_fd = initServer(argv[1], MAX_QUEUE);
	// Listen for connections from the clients
    // waitForConnections(server_fd, &bank_data, &data_locks);
    waitForConnections(server_fd, &viuda_data);

    printf("Closing the server socket\n");
    // Close the socket
    close(server_fd);

    // Clean the memory used
    // closeBank(&bank_data, &data_locks);

    printf("byeeeeee\n");
    // Finish the main thread
    pthread_exit(NULL);

    printf("ADIOS\n");

    return 0;
}

///// FUNCTION DEFINITIONS

/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char * program)
{
    printf("Usage:\n");
    printf("\t%s {port_number}\n", program);
    exit(EXIT_FAILURE);
}

void detectInterruption(int signal)
{
    printf("\nGot an interruption\n");
    interrupt_exit = 1;
}

/*
    Modify the signal handlers for specific events
*/
void setupHandlers()
{
    struct sigaction new_action;

    // Clear the structure
    bzero(&new_action, sizeof new_action);
    // Indicate the handler function to use
    new_action.sa_handler = detectInterruption;
    // Set a mask to block signals
    sigfillset(&new_action.sa_mask);

    // Establish the handler in my program
    sigaction(SIGINT, &new_action, NULL);

}



/*
    Function to initialize all the information necessary
    This will allocate memory for the accounts, and for the mutexes
*/
// void initBank(bank_t * bank_data, locks_t * data_locks)
// {
//     // Set the number of transactions
//     bank_data->total_transactions = 0;

//     // Allocate the arrays in the structures
//     bank_data->account_array = malloc(MAX_ACCOUNTS * sizeof (account_t));
//     // Allocate the arrays for the mutexes
//     data_locks->account_mutex = malloc(MAX_ACCOUNTS * sizeof (pthread_mutex_t));

//     // Initialize the mutexes, using a different method for dynamically created ones
//     //data_locks->transactions_mutex = PTHREAD_MUTEX_INITIALIZER;
//     pthread_mutex_init(&data_locks->transactions_mutex, NULL);
//     for (int i=0; i<MAX_ACCOUNTS; i++)
//     {
//         //data_locks->account_mutex[i] = PTHREAD_MUTEX_INITIALIZER;
//         pthread_mutex_init(&data_locks->account_mutex[i], NULL);
//         // Initialize the account balances too
//         bank_data->account_array[i].balance = 0.0;
//     }

//     // Read the data from the file
//     // readBankFile(bank_data);
// }


/*
    Get the data from the file to initialize the accounts
*/
void readBankFile(bank_t * bank_data)
{
    FILE * file_ptr = NULL;
    char buffer[BUFFER_SIZE];
    int account = 0;
    char * filename = "accounts.txt";

    file_ptr = fopen(filename, "r");
    if (!file_ptr)
    {
        perror("ERROR: fopen");
    }

    // Ignore the first line with the headers
    fgets(buffer, BUFFER_SIZE, file_ptr);
    // Read the rest of the account data
    while( fgets(buffer, BUFFER_SIZE, file_ptr) )
    {
        sscanf(buffer, "%d %d %f", &bank_data->account_array[account].id, &bank_data->account_array[account].pin, &bank_data->account_array[account].balance); 
        account++;
    }
    
    fclose(file_ptr);
}


/*
    Main loop to wait for incomming connections
*/
// void waitForConnections(int server_fd, bank_t * bank_data, locks_t * data_locks)
void waitForConnections(int server_fd, viuda_t * viuda_data)
{
    struct sockaddr_in client_address;
    socklen_t client_address_size;
    char client_presentation[INET_ADDRSTRLEN];
    int client_fd;
    // pthread_t new_tid;
    // int poll_response;
	// int timeout = 500;		// Time in milliseconds (0.5 seconds)
    // int connectionsNum = 0;
    int status;
    int idCount = 1; //Variable to assign ids for each player connected

    // Create a structure array to hold the file descriptors to poll
    // struct pollfd test_fds[1];
    // // Fill in the structure
    // test_fds[0].fd = server_fd;
    // test_fds[0].events = POLLIN;    // Check for incomming data

    while (1)
    {
        // if(interrupt_exit) {
        //     printf("Interrupted\n");
        //     break;
        // } 

        // poll_response = poll(test_fds, 1, timeout);
        // if (poll_response == 0)
        // {
        //     printf(". ");
        //     fflush(stdout);
        // }
        // else if (poll_response > 0) //if something was received
        // {
            client_address_size = sizeof client_address;

            // ACCEPT
            // Wait for a client connection
            client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_size);
            if (client_fd == -1)
            {
                perror("ERROR: accept");
                close(client_fd);
            }
            
            // Get the data from the client
            inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, sizeof client_presentation);
            printf("Received incomming connection from %s on port %d\n", client_presentation, client_address.sin_port);

            printf("CLIENT_FD: %d\n", client_fd);
            pthread_t tid;
            thread_data_t * connection_data = NULL;
            connection_data = malloc (sizeof (thread_data_t));
            // Prepare the structure to send to the thread
            connection_data->connection_fd = client_fd;
            connection_data->viuda_data = viuda_data;
            connection_data->playerId = idCount;
            
            // CREATE A THREAD
            status = pthread_create(&tid, NULL, attentionThread, connection_data);

            if (status == -1)
            {
                perror("ERROR: pthread_create");
                close(client_fd);
            }else {
                idCount++;
            }
        // }
        // else
        // {
        //     if (errno == EINTR) // if the poll gets interrupted, break while
        //     {
        //         printf("Error Interrupt\n");
        //         break;
                
        //     }
        //     else
        //     {
        //         perror("ERROR: poll");
        //         exit(EXIT_FAILURE);
        //     }
        // }
    }
    // Show the number of total transactions
    // printf("Transactions made: %d\n", bank_data->total_transactions);

    // free(connection_data);

    // Store any changes in the file
    // storeChanges(bank_data);
}

/*
    Hear the request from the client and send an answer
*/
void * attentionThread(void * arg)
{  
    // Receive the data for the bank, mutexes and socket file descriptor
    thread_data_t * info = arg;
    int round = 0;
    printf("\nSTARTED THREAD WITH CONNECTION: %d\n", info->connection_fd);
    // response_t response;
    // // Create a structure array to hold the file descriptors to poll
    // struct pollfd test_fds[1];
    // // Fill in the structure
    // test_fds[0].fd = info->connection_fd;
    // test_fds[0].events = POLLIN;    // Check for incomming data
    // int poll_response;
    // int timeout = 500;

    recvData(info->connection_fd, &info->message, sizeof info->message); //receive PLAY
    // Validate that the client corresponds to this server
    if (info->message.msg_code != PLAY)
    {
        printf("Error: unrecognized client\n");
        send(info->connection_fd, &info->message, sizeof info->message, 0);
        pthread_exit(NULL);
    } else {
        info->viuda_data->numPlayers++;
    }

    if(info->viuda_data->gameStatus == START){ //Game already started
        printf("Rejected client, the game has already started.\n");
        info->message.msg_code = FULL;
        info->viuda_data->numPlayers--;
        send(info->connection_fd, &info->message, sizeof info->message, 0);
        close(info->connection_fd);
        pthread_exit(NULL);
    }

    if(info->viuda_data->numPlayers>MAX_PLAYERS){  //Check if the maximum players (8) has been reached
        printf("Rejected client, there are already 8 players.\n");
        info->message.msg_code = FULL;
        info->viuda_data->numPlayers--;
        send(info->connection_fd, &info->message, sizeof info->message, 0);
        close(info->connection_fd);
        pthread_exit(NULL);
    }

    // Prepare a reply
    info->message.msg_code = AMOUNT;
    send(info->connection_fd, &info->message, sizeof info->message, 0);

    // Get the reply and validate again, receives AMOUNT
    recvData(info->connection_fd, &info->message, sizeof info->message);
    if (info->message.msg_code != AMOUNT)
    {
        printf("Error: unrecognized client\n");
        close(info->connection_fd);
        pthread_exit(NULL);
    }

    printf("The starting amount of the player is: %d\n", info->message.playerAmount);

    if((info->viuda_data->lowestAmount > info->message.playerAmount) || (info->viuda_data->lowestAmount == 0)) {
        info->viuda_data->lowestAmount = info->message.playerAmount;
    }

    printf("The players can bet at most %d.\n", info->viuda_data->lowestAmount);

    //add player to players array
    addNewPlayer(info->playerId, info->viuda_data);

    printf("Checking if the player is ready to start.\n");

    send(info->connection_fd, &info->message, sizeof info->message, 0);

    recvData(info->connection_fd, &info->message, sizeof info->message);

    //Conditional variable to know if all the players are ready to play
    if(info->message.theStatus == LOBBY) {
        pthread_mutex_lock(&ready_mutex);
          info->viuda_data->playersReady++;
          if (info->viuda_data->playersReady == info->viuda_data->numPlayers){
              info->viuda_data->gameStatus = START;
              assignTurns(info->viuda_data->players_array, info->viuda_data->numPlayers); //Put all the players at the beginning of the array so it is filled without empty spaces
              for(int i =0 ; i<info->viuda_data->numPlayers; i++){
                printf("PLAYER INFO: %d, Connected: %d, %d\n", info->viuda_data->players_array[i].id, info->viuda_data->players_array[i].connected, i);
              }
              printf("TESTING\n");
              pthread_cond_broadcast(&start_condition);
          } 
        pthread_mutex_unlock(&ready_mutex);
    }

    pthread_mutex_lock(&ready_mutex);
          while (info->viuda_data->playersReady < info->viuda_data->numPlayers) { //Do nothing (wait for this to be false)
                pthread_cond_wait(&start_condition, &ready_mutex);
          }
          info->message.lowestAmount = info->viuda_data->lowestAmount;
          printf("ESTABLISH LOWEST AMOUNT %d FOR THREAD WITH CONNECTION: %d", info->message.lowestAmount, info->connection_fd);
          
    pthread_mutex_unlock(&ready_mutex);

    // Prepare a reply
    info->message.playerStatus = START;
    info->message.dealerStatus = START;

    // printf("\tRunning thread with connection_fd: %d\n", info->connection_fd);

    // Loop to listen for messages from the client
    while(info->message.playerAmount >= 2){

        round++;

        printf("\n|||||||||||||||ROUND %d|||||||||||||||\n", round);

        // if(interrupt_exit) {
        //     printf("Interrupted\n");
        //     break;
        // } 

        // poll_response = poll(test_fds, 1, timeout);
        // if (poll_response == 0)
        // {
        //     printf(". ");
        //     fflush(stdout);
        // }
        // else if (poll_response > 0) //if something was received
        // {

            pthread_mutex_lock(&bets_mutex);
                while (info->viuda_data->players_array[info->viuda_data->index_playerInTurn].id != info->playerId){ 
                        pthread_cond_wait(&getBets_condition, &bets_mutex);
                }

                //Get the bet and player status from the client
                printf("\n/////Getting player %d bet/////\n", info->viuda_data->players_array[info->viuda_data->index_playerInTurn].id);
                printf("ENTRO AL SEGUNDO CONNECTION: %d\n", info->connection_fd);
                send(info->connection_fd, &info->message, sizeof info->message, 0);
                // Receive the request
                if (!recvData(info->connection_fd, &info->message, sizeof info->message)){
                    pthread_exit(NULL);
                    // return;
                }
                info->viuda_data->prize += info->message.playerBet;
                printf("The bet of the player is: %d\n", info->message.playerBet);    
            pthread_mutex_unlock(&bets_mutex);

            //Conditional variable to know the turn of the player to play
            pthread_mutex_lock(&bets_mutex);
            printf("CONNECTION: %d, CompararId %d, PLAYER IN TURN: %d, ESTE ID: %d\n", info->connection_fd, info->viuda_data->players_array[info->viuda_data->index_playerInTurn].id, info->viuda_data->index_playerInTurn, info->playerId);
                if (info->viuda_data->players_array[info->viuda_data->index_playerInTurn].id == info->playerId){
                    printf("ENTRO AL PRIMERO CONNECTION: %d\n", info->connection_fd);
                    pthread_mutex_lock(&betsReady_mutex);
                    printf("info->viuda_data->index_playerInTurn: %d\n", info->viuda_data->index_playerInTurn);
                    info->viuda_data->index_playerInTurn++;
                    if (info->viuda_data->index_playerInTurn == info->viuda_data->numPlayers){
                        printf("The prize for the winner is: %d\n", info->viuda_data->prize);
                        dealCards(info->viuda_data);
                        chooseViuda(info->viuda_data);
                        pthread_cond_broadcast(&betsReady_condition);
                    } 
                    pthread_mutex_unlock(&betsReady_mutex);
                    printf("UNLOCKED\n");
                    pthread_cond_broadcast(&getBets_condition);
                } 
            pthread_mutex_unlock(&bets_mutex);

            pthread_mutex_lock(&betsReady_mutex);
                while (info->viuda_data->index_playerInTurn < info->viuda_data->numPlayers){ //Do nothing (wait for ths to be false)
                        pthread_cond_wait(&betsReady_condition, &betsReady_mutex);
                }
            pthread_mutex_unlock(&betsReady_mutex);                 
            
            
            completeFirstDeal(&info->message, info->connection_fd); //Generates the first 2 cards of the Player and Dealer

            if((info->message.dealerStatus != NATURAL) && (info->message.playerStatus != NATURAL)){ // if no one got a natural blackjack
                printf("\n/////PLAYER'S TURN/////\n");
                playerTurn(&info->message, info->connection_fd); //Update player info based on the options chosen by the user

                //Calculate dealers hands and total accumulated
                printf("\n/////DEALER'S TURN/////\n");
                dealerTurn(&info->message, info->connection_fd); //Update dealer's info when player's turn is over
            }

            //Take decision based on the status 
            printf("\n/////TAKING DECISION BASED ON THE STATUS/////\n");
            calculateResults(&info->message);

            if(info->message.playerAmount < 2){ //The client disconnects when the player doesn't have enough chips
                printf("The player doesn't have enough money to keep playing. The player will exit now.\n");
            } 

            send(info->connection_fd, &info->message, sizeof info->message, 0);


    //         // Update the number of transactions
    //         if(response == OK){
    //             pthread_mutex_lock(&info->data_locks->transactions_mutex);
    //             info->bank_data->total_transactions++;
    //             pthread_mutex_unlock(&info->data_locks->transactions_mutex);
    //         } else if(response == NO_ACCOUNT){
    //             printf("Invalid acount number received.\n");
    //         }

    //         // Prepare the message to the client
    //         sprintf(buffer, "%d %f", response, balance);

    //         // Send a reply
    //         sendData(info->connection_fd, buffer, strlen(buffer)+1);
    //     }
    //     else
    //     {
    //         if (errno == EINTR) // if the poll gets interrupted, break while
    //         {
    //             printf("Error Interrupt\n");
    //             break;
                
    //         }
    //         else
    //         {
    //             perror("ERROR: poll");
    //             exit(EXIT_FAILURE);
    //         }
    //     }
    }

    printf("SALIOO\n");

    printf("\nENDING THREAD WITH CONNECTION: %d\n", info->connection_fd);

    // Finish the connection
    if (!recvData(info->connection_fd, &info->message, sizeof info->message))
    {
        printf("DENTRO DE FUNCION\n");
        printf("Client disconnected\n");
    }
    info->message.msg_code = BYE;
    send(info->connection_fd, &info->message, sizeof info->message, 0);
    close(info->connection_fd);

    pthread_exit(NULL);
}

void dealCards(viuda_t * viuda_data){

    const char cardsArray[13][3]= {"A","2","3","4","5","6","7","8","9","10","J","Q","K"};
    const int valuesArray[13]= {14,2,3,4,5,6,7,8,9,10,11,12,13};
    const char suitsArray[4][9]= {"Hearts","Diamonds","Clubs","Spades"}; 
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
    for(int i = 0; i < 52; i++) {
        printf("#%d, Card %s, Value %d, Suit %s, SuitValue %d Used? %d\n", i+1, cards[i].rank, cards[i].rank_value, cards[i].suit, cards[i].suit_value, cards[i].used);
    }

    //generate players cards
    for(int i = 0; i<viuda_data->numPlayers; i++){
        given_cards = 0;
        while(given_cards < 5){
            random_number = rand() % 52;
            random_card = cards[random_number];
            if(random_card.used){
                continue;
            } else {
                strcpy(viuda_data->players_array[i].hand.cards[given_cards].rank, random_card.rank);
                strcpy(viuda_data->players_array[i].hand.cards[given_cards].suit, random_card.suit);
                viuda_data->players_array[i].hand.cards[given_cards].rank_value = cards[random_number].rank_value;
                viuda_data->players_array[i].hand.cards[given_cards].suit_value = cards[random_number].suit_value;
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
            strcpy(viuda_data->table_hand.cards[given_cards].rank, random_card.rank);
            strcpy(viuda_data->table_hand.cards[given_cards].suit, random_card.suit);
            viuda_data->table_hand.cards[given_cards].rank_value = random_card.rank_value;
            viuda_data->table_hand.cards[given_cards].suit_value = random_card.suit_value;
            cards[random_number].used = 1;
            given_cards++;
        }
    }

    //print player cards
    for(int i = 0; i<((viuda_data->numPlayers) + 1); i++){
        if(i == viuda_data->numPlayers){
            printf("Table cards:\n");
            for(int j = 0; j<5; j++){
                printf("Cart: %s, Suit %s, SuitValue: %d, Value %d\n", viuda_data->table_hand.cards[j].rank, viuda_data->table_hand.cards[j].suit, viuda_data->table_hand.cards[j].suit_value, viuda_data->table_hand.cards[j].rank_value);
            }
            printf("\n");
        }else {
            printf("Player: %d\n", viuda_data->players_array[i].id);
            for(int j = 0; j<5; j++){
                printf("Cart: %s, Suit %s, SuitValue: %d, Value %d\n", viuda_data->players_array[i].hand.cards[j].rank, viuda_data->players_array[i].hand.cards[j].suit, viuda_data->players_array[i].hand.cards[j].suit_value, viuda_data->players_array[i].hand.cards[j].rank_value);
            }
            printf("\n");
        }
    }

    //Print cards
    for(int i = 0; i < 52; i++) {
        printf("#%d, Card %s, Suit %s, SuitValue: %d, Used? %d, Value %d\n", i+1, cards[i].rank, cards[i].suit, cards[i].suit_value, cards[i].used, cards[i].rank_value);
    }

    //print player cards
    for(int i = 0; i<((viuda_data->numPlayers) + 1); i++){
        if(i == viuda_data->numPlayers){
            sortCardsByRank(&viuda_data->table_hand);
        }else {
            sortCardsByRank(&viuda_data->players_array[i].hand);
        }
    }

    //print player cards
    for(int i = 0; i<((viuda_data->numPlayers) + 1); i++){
        if(i == viuda_data->numPlayers){
            printf("Sorted Table cards:\n");
            for(int j = 0; j<5; j++){
                printf("Cart: %s, Suit %s, SuitValue %d,Value %d\n", viuda_data->table_hand.cards[j].rank, viuda_data->table_hand.cards[j].suit, viuda_data->table_hand.cards[j].suit_value, viuda_data->table_hand.cards[j].rank_value);
            }
            printf("\n");
        }else {
            printf("Sorted Player %d Cards\n", viuda_data->players_array[i].id);
            for(int j = 0; j<5; j++){
                printf("Cart: %s, Suit %s, SuitValue %d, Value %d\n", viuda_data->players_array[i].hand.cards[j].rank, viuda_data->players_array[i].hand.cards[j].suit, viuda_data->players_array[i].hand.cards[j].suit_value, viuda_data->players_array[i].hand.cards[j].rank_value);
            }
            printf("\n");
        }
    }

    printf("\n\nTESTING EVALUATION \n\n");

    for(int i = 0; i<((viuda_data->numPlayers) + 1); i++){
        if(i == viuda_data->numPlayers){
            
            printf("Evaluated table\n");
            for(int j = 0; j<5; j++){
                printf("Cart: %s, Suit %s, SuitValue %d,Value %d\n", viuda_data->table_hand.cards[j].rank, viuda_data->table_hand.cards[j].suit, viuda_data->table_hand.cards[j].suit_value, viuda_data->table_hand.cards[j].rank_value);
            }
            evaluateHand(&viuda_data->table_hand);
            printf("Type: %d, Untie Value: %d, HighCardRank: %s,  HighCardValue: %d\n", viuda_data->table_hand.type, viuda_data->table_hand.total_value, viuda_data->table_hand.high_card_rank, viuda_data->table_hand.high_card_value);
        }else {
            
            printf("Evaluated player %d \n", viuda_data->players_array[i].id);
            for(int j = 0; j<5; j++){
                printf("Cart: %s, Suit %s, SuitValue %d, Value %d\n", viuda_data->players_array[i].hand.cards[j].rank, viuda_data->players_array[i].hand.cards[j].suit, viuda_data->players_array[i].hand.cards[j].suit_value, viuda_data->players_array[i].hand.cards[j].rank_value);
            }
            evaluateHand(&viuda_data->players_array[i].hand);
            printf("Type: %d, Untie Value: %d, HighCardRank: %s,  HighCardValue: %d\n", viuda_data->players_array[i].hand.type, viuda_data->players_array[i].hand.total_value, viuda_data->players_array[i].hand.high_card_rank,viuda_data->players_array[i].hand.high_card_value);
        }
    }

    compareHands(viuda_data->table_hand, viuda_data->players_array[0].hand);
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

void chooseViuda (viuda_t *viuda_data){
    
    int option;
    int chooseCard;
    int chooseTableCard;
    //full hand
    hand_t playerHand = viuda_data->players_array[0].hand;
    hand_t tableHand = viuda_data->table_hand;
    hand_t temp;

    //one card
    card_t tempCard;

    while(1){ //Keep asking for a valid bet
            printf("Si quieres la viuda entera presiona 1, si quieres solo uan carta presiona 2\n");
            scanf("%d", &option);
            if(option == 1){
                printf("Toma la viuda entera");
                temp = playerHand;
                playerHand = tableHand;
                tableHand = temp;
                viuda_data->table_hand = tableHand;
                viuda_data->players_array[0].hand=playerHand;
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
                viuda_data->table_hand = tableHand;
                viuda_data->players_array[0].hand=playerHand;
                break;
            }
            else {
               printf("Opcion no valida, vuelve a intentar");
            }
    }
    for(int i = 0; i<((viuda_data->numPlayers) + 1); i++){
        if(i == viuda_data->numPlayers){
            printf("Changed Table cards:\n");
            for(int j = 0; j<5; j++){
                printf("Cart: %s, Suit %s, SuitValue %d,Value %d\n", viuda_data->table_hand.cards[j].rank, viuda_data->table_hand.cards[j].suit, viuda_data->table_hand.cards[j].suit_value, viuda_data->table_hand.cards[j].rank_value);
            }
            printf("\n");
        }else {
            printf("Changed Player %d Cards\n", viuda_data->players_array[i].id);
            for(int j = 0; j<5; j++){
                printf("Cart: %s, Suit %s, SuitValue %d, Value %d\n", viuda_data->players_array[i].hand.cards[j].rank, viuda_data->players_array[i].hand.cards[j].suit, viuda_data->players_array[i].hand.cards[j].suit_value, viuda_data->players_array[i].hand.cards[j].rank_value);
            }
            printf("\n");
        }
    }
}

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
        printf("SAL\n");
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

void addNewPlayer(int playerId, viuda_t * viuda_data) {

    for(int i = 0; i<MAX_PLAYERS; i++){
        if(viuda_data->players_array[i].id == 0){
            viuda_data->players_array[i].id = playerId;
            viuda_data->players_array[i].connected = 1;
            viuda_data->players_array[i].lost = 0;
            viuda_data->players_array[i].status = LOBBY;
            printf("Added player with ID: %d\n", viuda_data->players_array[i].id);
            break;
        }
    }

}

void removePlayer(int playerId, viuda_t * viuda_data) {

    for(int i = 0; i<MAX_PLAYERS; i++){
        if(viuda_data->players_array[i].id == playerId){
            viuda_data->players_array[i].id = 0;
            viuda_data->players_array[i].connected = 0;
            printf("Removed player with ID: %d\n", viuda_data->players_array[i].id);
            break;
        }
    }

}

int getRandomCard(message_t * message, char pord){ //pord stands for player or dealer

    int newCard;
    char randomCard[3]; 

    const char cardsArray[13][3]= {"A","2","3","4","5","6","7","8","9","10","J","Q","K"};

    strcpy(randomCard, cardsArray[rand() % 13]); //Pick a random card from the array

    if(strcmp(randomCard, "A") == 0){
        newCard = 11; //Ace is equal to 11
    } else if ((strcmp(randomCard, "10") == 0) || (strcmp(randomCard, "J") == 0) || (strcmp(randomCard, "Q") == 0) || (strcmp(randomCard, "K") == 0)) {
        newCard = 10; //10, J, Q, and K are equal to 10
    } else {
        newCard = atol(randomCard); //Convert string to integer
    }

    if(pord == 'p'){ //If the player called the function
        strcpy((message->playerCards)[message->numPlayerCards], randomCard);
        if(message->numPlayerCards >= 2){
            printf("New card is: [%s].", randomCard);
        }
        (message->numPlayerCards)++;
        if((newCard == 11) && (message->totalPlayer + 11 > 21)){ //If ace + current amount > 21, then ace value is 1
            newCard = 1;
        }
    } else if(pord == 'd'){ //If the dealer called the function
        if(message->numDealerCards >= 2){
            printf("Dealer gets new card: [%s].", randomCard);
        }
        strcpy((message->dealerCards)[message->numDealerCards], randomCard);
        (message->numDealerCards)++;
        if((newCard == 11) && (message->totalDealer + 11 > 21)){ //If ace + current amount > 21, then ace value is 1
            newCard = 1;
        }
    }

    return newCard;
}

//Generates the first 2 cards of the Player and Dealer and tells if someone got a Natural Blackjack
void completeFirstDeal(message_t * message, int connection_fd){

    //Reset number of cards of the player and dealer from the previous round
    message->numPlayerCards = 0;
    message->numDealerCards = 0;
 
    message->totalPlayer = 0;
    message->totalDealer = 0;

    srand(time(NULL));

    for(int i = 0; i<2; i++){
        message->totalPlayer += getRandomCard(message, 'p');
        message->totalDealer += getRandomCard(message, 'd');
    }

    //Check for Natural blackjacks
    if(message->totalPlayer == 21) {
        printf("The player got a Natural Blackjack with the cards [%s] and [%s]!\n", message->playerCards[0], message->playerCards[1]);
        message->playerStatus = NATURAL;
    }

    if(message->totalDealer == 21) {
        message->dealerStatus = NATURAL;
        printf("The dealer got a Natural Blackjack with the cards [%s] and [%s]!\n", message->dealerCards[0], message->dealerCards[1]);
    }

    //If there is send the status to the client
    if((message->totalPlayer == 21) || (message->totalDealer == 21)) {
        send(connection_fd, message, sizeof (*message), 0);
    }

}

void playerTurn(message_t * message, int connection_fd){

    int newCard = 0;

    //Initial deal
    printf("Initial hand of the player: [%s] [%s] ", message->playerCards[0], message->playerCards[1]);
    printf("summing a total of: %d\n", message->totalPlayer);

    //Sends the total hand accumulated by the player
    send(connection_fd, message, sizeof (*message), 0);
    //Gets the status chosen by the player
    recvData(connection_fd, message, sizeof (* message));
    // Send the corresponding reply
    while (message->playerStatus == HIT)
    {
        printf("The player with a total of %d chose to get a card.\n", message->totalPlayer);
        newCard = getRandomCard(message, 'p');
        message->totalPlayer += newCard;
        printf(" The new total of this player is: %d.\n", message->totalPlayer);

        if (message->totalPlayer == 21) {
            message->playerStatus = TWENTYONE;
            printf("The current player got 21!\n");
            send(connection_fd, message, sizeof (*message), 0);
            break;
        } else if (message->totalPlayer > 21) {
            message->playerStatus = BUST;
            printf("The current player busted, he is over 21.\n");
            send(connection_fd, message, sizeof (*message), 0);
            break;
        }

        //Sends the status calculated by the server
        send(connection_fd, message, sizeof (*message), 0);

        //Gets the status chosen by the player
        recvData(connection_fd, message, sizeof (*message));

        if(message->playerStatus == STAND) {
            printf("Player chose to stay with %d.\n", message->totalPlayer);
        }
    }

    printf("After completing his/her turn the player accumulated the cards:");
    for(int i = 0; i<message->numPlayerCards; i++){
        printf(" [%s]", message->playerCards[i]);
    }
    printf(" which sum a total of: %d\n", message->totalPlayer);
}

void dealerTurn(message_t * message, int connection_fd){ //Automatic deicisions based on Blackjack rules

    int newCard = 0;

    //if the player turn is over
    if((message->playerStatus == STAND) || (message->playerStatus == NATURAL) || (message->playerStatus == TWENTYONE)) {

        printf("Initial dealer's hand: [%s] [%s] making a total of: %d\n", message->dealerCards[0], message->dealerCards[1], message->totalDealer);

        message->dealerStatus = HIT;
        while((message->dealerStatus != STAND) && (message->dealerStatus != BUST) && (message->dealerStatus != TWENTYONE)){
            if((message->totalDealer >= 17) && (message->totalDealer < 21)){
                printf("The dealer stays with a total of: %d\n", message->totalDealer);
                message->dealerStatus = STAND;
            } else if (message->totalDealer > 21) {
                printf("The dealer exceeds 21 with %d and busts.\n", message->totalDealer);
                message->dealerStatus = BUST;
            } else if(message->totalDealer == 21) {
                printf("The dealer got %d!\n", message->totalDealer);
                message->dealerStatus = TWENTYONE;
            } else {
                newCard = getRandomCard(message, 'd'); //If dealer status == HIT
                message->totalDealer += newCard;
                printf(" New dealer's total: %d\n", message->totalDealer);
            }
        }
    }

    printf("After completing the turn dealer accumulated the cards:");
    for(int i = 0; i<message->numDealerCards; i++){
        printf(" [%s]", message->dealerCards[i]);
    }
    printf(" which sum a total of: %d\n", message->totalDealer);
}

void calculateResults(message_t * message){

    if (message->playerStatus == BUST){
        printf("The dealer gets the player's bet.\n");
        message->playerAmount -= message->playerBet;
    } else if((message->dealerStatus == BUST) && (message->playerStatus == STAND)){
        printf("The dealer gives the player his bet plus the amount of his bet.\n");
        message->playerAmount += message->playerBet;
    } else if (((message->dealerStatus == STAND) || (message->dealerStatus == TWENTYONE)) && ((message->playerStatus == STAND) || (message->playerStatus == TWENTYONE))){
        if(message->totalPlayer > message->totalDealer){
            printf("The dealer gives the player his bet plus the amount of his bet.\n");
            message->playerAmount += message->playerBet;
        }else if (message->totalPlayer < message->totalDealer) {
            printf("The dealer gets the player's bet.\n");
            message->playerAmount -= message->playerBet;
        } else if (message->totalPlayer == message->totalDealer){
            printf("This is stand-off. The player gets his bet back.\n");
        }
    } else if ((message->dealerStatus == NATURAL) && (message->playerStatus != NATURAL)){
        printf("The dealer gets the player's bet.\n");
        message->playerAmount -= message->playerBet;
    } else if ((message->dealerStatus != NATURAL) && (message->playerStatus == NATURAL)){
        printf("The dealer gives the player his bet plus 1.5x the amount of his bet.\n");
        message->playerAmount += (message->playerBet * 1.5);
    } else if ((message->dealerStatus == NATURAL) && (message->playerStatus == NATURAL)){
        printf("This is PUSH. The player gets his bet back.\n");
    }
}

/*
    Function to initialize all the information necessary
*/
void initGame(viuda_t * viuda_data) {

    viuda_data->numPlayers = 0;
    viuda_data->gameBet = 0;
    viuda_data->prize = 0;
    viuda_data->winner = 0;
    viuda_data->playerInTurn = 0;
    viuda_data->index_playerInTurn = 0;
    viuda_data->playersReady =  0;
    viuda_data->lowestAmount =  0;
    viuda_data->gameStatus = WAIT;

    viuda_data->players_array = malloc(MAX_PLAYERS * sizeof (player_t));

    for(int i = 0; i<MAX_PLAYERS; i++){
        viuda_data->players_array[i].id = 0;
        viuda_data->players_array[i].status = WAIT;
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
/*
    Free all the memory used for the bank data
*/
// void closeBank(bank_t * bank_data, locks_t * data_locks)
// {
//     printf("DEBUG: Clearing the memory for the thread\n");
//     free(bank_data->account_array);
//     free(data_locks->account_mutex);
// }


/*
    Return true if the account provided is within the valid range,
    return false otherwise
*/
int checkValidAccount(int account)
{
    return (account >= 0 && account < MAX_ACCOUNTS);
}
