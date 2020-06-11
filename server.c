 
/*
    Emiliano Peredo A01422326
    Raziel Nicolás Martínez Castillo A01410695
    Camila Rovirosa Ochoa A01024192

    Proyect: Whiskey Poker

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
#include "whiskey.h"
#include "sockets.h"


#define MAX_ACCOUNTS 5
#define BUFFER_SIZE 1024
#define MAX_QUEUE 5


///// Structure definitions

// Data that will be sent to each thread
typedef struct data_struct {
    // The file descriptor for the socket
    message_t message;
    int connection_fd;
    int playerId;
    
    whiskey_t * whiskey_data;
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

pthread_cond_t roundReady_condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t roundReady_mutex = PTHREAD_MUTEX_INITIALIZER;

///// FUNCTION DECLARATIONS
void usage(char * program);
void setupHandlers();
void detectInterruption(int signal);

void waitForConnections(int server_fd, whiskey_t * whiskey_data);
void * attentionThread(void * arg);

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int server_fd;

    whiskey_t whiskey_data;

    printf("\n=== SERVER PROGRAM ===\n");

    // Check the correct arguments
    if (argc != 2)
    {
        usage(argv[0]);
    }

    // Configure the handler to catch SIGINT
    // setupHandlers();

    // Initialize the data structures
    initGame(&whiskey_data);

	// Show the IPs assigned to this computer
	printLocalIPs();
    // Start the server
    server_fd = initServer(argv[1], MAX_QUEUE);
	// Listen for connections from the clients
    waitForConnections(server_fd, &whiskey_data);

    printf("Closing the server socket\n");
    // Close the socket
    close(server_fd);

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
    Main loop to wait for incomming connections
*/
void waitForConnections(int server_fd, whiskey_t * whiskey_data)
{
    struct sockaddr_in client_address;
    socklen_t client_address_size;
    char client_presentation[INET_ADDRSTRLEN];
    int client_fd;
    int status;
    int idCount = 1; //Variable to assign ids for each player connected

    while (1)
    {
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
            connection_data->whiskey_data = whiskey_data;
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
    }

}

/*
    Hear the request from the client and send an answer
*/
void * attentionThread(void * arg)
{  
    // Receive the data for the thread, mutexes and socket file descriptor
    thread_data_t * info = arg;
    int round = 0;
    printf("\nSTARTED THREAD WITH CONNECTION: %d\n", info->connection_fd);

    recvData(info->connection_fd, &info->message, sizeof info->message); //receive PLAY
    // Validate that the client corresponds to this server
    if (info->message.msg_code != PLAY)
    {
        printf("Error: unrecognized client\n");
        send(info->connection_fd, &info->message, sizeof info->message, 0);
        pthread_exit(NULL);
    } else {
        info->whiskey_data->numPlayers++;
    }

    if(info->whiskey_data->gameStatus == START){ //Game already started
        printf("Rejected client, the game has already started.\n");
        info->message.msg_code = FULL;
        info->whiskey_data->numPlayers--;
        send(info->connection_fd, &info->message, sizeof info->message, 0);
        close(info->connection_fd);
        pthread_exit(NULL);
    }

    if(info->whiskey_data->numPlayers>MAX_PLAYERS){  //Check if the maximum players (8) has been reached
        printf("Rejected client, there are already 8 players.\n");
        info->message.msg_code = FULL;
        info->whiskey_data->numPlayers--;
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

    if((info->whiskey_data->lowestAmount > info->message.playerAmount) || (info->whiskey_data->lowestAmount == 0)) {
        info->whiskey_data->lowestAmount = info->message.playerAmount;
    }

    printf("The players can bet at most %d.\n", info->whiskey_data->lowestAmount);

    //add player to players array
    addNewPlayer(info->playerId, info->whiskey_data);
    info->message.playerLives = 3;

    printf("Checking if the player is ready to start.\n");

    send(info->connection_fd, &info->message, sizeof info->message, 0);
    //RECEIVES START
    recvData(info->connection_fd, &info->message, sizeof info->message);

    //Conditional variable to know if all the players are ready to play
    if(info->message.theStatus == LOBBY) {
        pthread_mutex_lock(&ready_mutex);
          info->whiskey_data->playersReady++;
          if (info->whiskey_data->playersReady == info->whiskey_data->numPlayers){
              info->whiskey_data->gameStatus = START;
              assignTurns(info->whiskey_data->players_array, info->whiskey_data->numPlayers); //Put all the players at the beginning of the array so it is filled without empty spaces
              for(int i =0 ; i<info->whiskey_data->numPlayers; i++){
                printf("PLAYER INFO: %d, Connected: %d, %d\n", info->whiskey_data->players_array[i].id, info->whiskey_data->players_array[i].connected, i);
              }
              printf("TESTING\n");
              pthread_cond_broadcast(&start_condition);
          } 
        pthread_mutex_unlock(&ready_mutex);
    }

    pthread_mutex_lock(&ready_mutex);
          while (info->whiskey_data->playersReady < info->whiskey_data->numPlayers) { //Do nothing (wait for this to be false)
                pthread_cond_wait(&start_condition, &ready_mutex);
          }
          info->message.lowestAmount = info->whiskey_data->lowestAmount;
          printf("ESTABLISH LOWEST AMOUNT %d FOR THREAD WITH CONNECTION: %d", info->message.lowestAmount, info->connection_fd);
          
    pthread_mutex_unlock(&ready_mutex);

    // Prepare a reply
    info->message.playerStatus = START;
    

    //Send OK
    send(info->connection_fd, &info->message, sizeof info->message, 0);

    // printf("\tRunning thread with connection_fd: %d\n", info->connection_fd);

    // Loop to listen for messages from the client
    while(info->whiskey_data->numPlayers >= 2){

        round++;
        dealCards(info->whiskey_data);

        printf("\n|||||||||||||||ROUND %d|||||||||||||||\n", round);
        
        int knocked = 0;

        info->whiskey_data->turn = 0;

        while(!knocked){

            info->whiskey_data->turn++;

            printf("NUEVA RONDA\n");

            printf("CONNECTION: %d, CompararId %d, PLAYER IN TURN: %d, ESTE ID: %d\n", info->connection_fd, info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].id, info->whiskey_data->index_playerInTurn, info->playerId);

            printf("Sin recibir 1\n");

            recvData(info->connection_fd, &info->message, sizeof info->message);
            
            printf("Recibido 1\n");

            send(info->connection_fd, &info->message, sizeof info->message, 0);

            printf("ENVIADO ESTE ID: %d\n", info->playerId);

            recvData(info->connection_fd, &info->message, sizeof info->message);

            printf("3 RECIBIDO \n");

            pthread_mutex_lock(&roundReady_mutex);
            info->whiskey_data->index_startRoundIndex++;
            printf("ROUND INDEX %d\n", info->whiskey_data->index_startRoundIndex);
            if (info->whiskey_data->index_startRoundIndex == info->whiskey_data->numPlayers){
                info->whiskey_data->index_playerInTurn = 0;
                printf("ROUND_PLAYER INFO: %d\n", info->playerId);
                printf("TESTING\n");
                pthread_cond_broadcast(&roundReady_condition);
            } 
            pthread_mutex_unlock(&roundReady_mutex);


            pthread_mutex_lock(&roundReady_mutex);
                while (info->whiskey_data->index_startRoundIndex < info->whiskey_data->numPlayers) { //Do nothing (wait for this to be false)
                        pthread_cond_wait(&roundReady_condition, &roundReady_mutex);
                }
            pthread_mutex_unlock(&roundReady_mutex);

            pthread_mutex_lock(&bets_mutex);
                while (info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].id != info->playerId){ 
                        pthread_cond_wait(&getBets_condition, &bets_mutex);
                }

                //Get the bet and player status from the client
                printf("\n/////Getting player %d bet/////\n", info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].id);
                printf("ENTERING SECOND CONNECTION: %d\n", info->connection_fd);

                info->message.whiskeyHand = info->whiskey_data->table_hand;
                printf("\n/////SENDING HAND TO PLAYER %d/////\n", info->whiskey_data->index_playerInTurn);
                printHand(info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].hand);
                info->message.playerHand = info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].hand;

                info->message.turn = info->whiskey_data->turn;

                send(info->connection_fd, &info->message, sizeof info->message, 0);
                // Receive the request
                if (!recvData(info->connection_fd, &info->message, sizeof info->message)){
                    pthread_exit(NULL);
                    // return;
                }
                //Response handling
                printf("RESPONSE: %d\n", info->message.playerStatus);

                switch (info->message.playerStatus)
                {
                case KNOCK:
                    knocked = 1; 
                    break;
                case CHANGE_ONE:
                    info->whiskey_data->table_hand = info->message.whiskeyHand;

                    info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].hand = info->message.playerHand;

                    printf("NEW TABLE HAND\n");
                    printHand( info->whiskey_data->table_hand);
                    printf("NEW PLAYER %d HAND\n", info->whiskey_data->index_playerInTurn);
                    printHand(info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].hand);
                    break;
                case CHANGE_ALL:
                    info->whiskey_data->table_hand = info->message.whiskeyHand;

                    info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].hand = info->message.playerHand;

                    //Updates turn for unhidding table hand in case of changing all cards
                    info->whiskey_data->turn++;
                    
                    printf("NEW TABLE HAND\n");
                    printHand( info->whiskey_data->table_hand);
                    printf("NEW PLAYER %d HAND\n", info->whiskey_data->index_playerInTurn);
                    printHand(info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].hand);
                    break;
                case PASS:
                    break;
                default:
                    break;
                }
                info->whiskey_data->prize += info->message.playerBet;
                printf("The bet of the player is: %d\n", info->message.playerBet);    
            pthread_mutex_unlock(&bets_mutex);
            
            //Conditional variable to know the turn of the player to play
            pthread_mutex_lock(&bets_mutex);
            printf("CONNECTION: %d, CompararId %d, PLAYER IN TURN: %d, ESTE ID: %d\n", info->connection_fd, info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].id, info->whiskey_data->index_playerInTurn, info->playerId);
                if (info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].id == info->playerId){
                    printf("ENTERING FIRST CONNECTION: %d\n", info->connection_fd);
                    pthread_mutex_lock(&betsReady_mutex);
                    printf("info->whiskey_data->index_playerInTurn: %d\n", info->whiskey_data->index_playerInTurn);
                    info->whiskey_data->index_playerInTurn++;
                    if (info->whiskey_data->index_playerInTurn == info->whiskey_data->numPlayers){
                        printf("THE GREATE PRIZE IS: %d\n", info->whiskey_data->prize);
                        info->whiskey_data->index_startRoundIndex = 0;
                        printf("Going to next round\n");
                        // dealCards(info->whiskey_data);
                        // chooseViuda(info->whiskey_data);
                        pthread_cond_broadcast(&betsReady_condition);
                    } 
                    pthread_mutex_unlock(&betsReady_mutex);
                    printf("UNLOCKED\n");
                    pthread_cond_broadcast(&getBets_condition);
                } 
            pthread_mutex_unlock(&bets_mutex);

            pthread_mutex_lock(&betsReady_mutex);
                while (info->whiskey_data->index_playerInTurn < info->whiskey_data->numPlayers){ //Do nothing (wait for ths to be false)
                    pthread_cond_wait(&betsReady_condition, &betsReady_mutex);
                }
            pthread_mutex_unlock(&betsReady_mutex);       


        }
        //END LOOP

        printf("FINAL ROUND\n");

        printf("CONNECTION: %d, CompararId %d, PLAYER IN TURN: %d, ESTE ID: %d\n", info->connection_fd, info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].id, info->whiskey_data->index_playerInTurn, info->playerId);

        printf("Nothing was recieved 1\n");

        recvData(info->connection_fd, &info->message, sizeof info->message);
        
        printf("Recived 1\n");

        send(info->connection_fd, &info->message, sizeof info->message, 0);

        printf("Sending this ID: %d\n", info->playerId);

        recvData(info->connection_fd, &info->message, sizeof info->message);

        printf("3 RECIVED \n");

        pthread_mutex_lock(&roundReady_mutex);
        info->whiskey_data->index_startRoundIndex++;
        printf("ROUND INDEX %d\n", info->whiskey_data->index_startRoundIndex);
        if (info->whiskey_data->index_startRoundIndex == info->whiskey_data->numPlayers){
            info->whiskey_data->index_playerInTurn = 0;
            printf("ROUND_PLAYER INFO: %d\n", info->playerId);
            printf("TESTING\n");
            pthread_cond_broadcast(&roundReady_condition);
        } 
        pthread_mutex_unlock(&roundReady_mutex);


        pthread_mutex_lock(&roundReady_mutex);
            while (info->whiskey_data->index_startRoundIndex < info->whiskey_data->numPlayers) { //Do nothing (wait for this to be false)
                    pthread_cond_wait(&roundReady_condition, &roundReady_mutex);
            }
        pthread_mutex_unlock(&roundReady_mutex);

        pthread_mutex_lock(&bets_mutex);
            while (info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].id != info->playerId){ 
                    pthread_cond_wait(&getBets_condition, &bets_mutex);
            }

            //Get the bet and player status from the client
            printf("\n/////Getting player %d bet/////\n", info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].id);
            printf("ENTERING SECOND CONNECTION: %d\n", info->connection_fd);

            info->message.whiskeyHand = info->whiskey_data->table_hand;
            printf("\n/////SENDING HAND/////%d\n", info->whiskey_data->index_playerInTurn);
            printHand(info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].hand);
            info->message.playerHand = info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].hand;

            send(info->connection_fd, &info->message, sizeof info->message, 0);
            // Receive the request
            if (!recvData(info->connection_fd, &info->message, sizeof info->message)){
                pthread_exit(NULL);
                // return;
            }
            //Response handling
            printf("RESPONSE: %d\n", info->message.playerStatus);

            switch (info->message.playerStatus)
            {
            case KNOCK:
                
                break;
            case CHANGE_ONE:
                info->whiskey_data->table_hand = info->message.whiskeyHand;

                info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].hand = info->message.playerHand;

                printf("NEW TABLE HAND\n");
                printHand( info->whiskey_data->table_hand);
                printf("NEW PLAYER %d HAND\n", info->whiskey_data->index_playerInTurn);
                printHand(info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].hand);
                break;
            case CHANGE_ALL:
            
                info->whiskey_data->table_hand = info->message.whiskeyHand;

                info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].hand = info->message.playerHand;

                printf("NEW TABLE HAND\n");
                printHand( info->whiskey_data->table_hand);
                printf("NEW PLAYER %d HAND\n", info->whiskey_data->index_playerInTurn);
                printHand(info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].hand);
                break;
            default:
                break;
            }
            info->whiskey_data->prize += info->message.playerBet;
            printf("The bet of the player is: %d\n", info->message.playerBet);    
        pthread_mutex_unlock(&bets_mutex);

        //Conditional variable to know the turn of the player to play
        pthread_mutex_lock(&bets_mutex);
        printf("CONNECTION: %d, CompararId %d, PLAYER IN TURN: %d, ESTE ID: %d\n", info->connection_fd, info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].id, info->whiskey_data->index_playerInTurn, info->playerId);
            if (info->whiskey_data->players_array[info->whiskey_data->index_playerInTurn].id == info->playerId){
                printf("ENTRO AL PRIMERO CONNECTION: %d\n", info->connection_fd);
                pthread_mutex_lock(&betsReady_mutex);
                printf("info->whiskey_data->index_playerInTurn: %d\n", info->whiskey_data->index_playerInTurn);
                info->whiskey_data->index_playerInTurn++;
                if (info->whiskey_data->index_playerInTurn == info->whiskey_data->numPlayers){
                    info->whiskey_data->index_startRoundIndex = 0;
                    printf("Going to next round\n");
                    //CHOOSE LOOSER
                    int looser_index = 0;
                    for(int i = 1; i< info->whiskey_data->numPlayers; i++){
                        if(!firstHandIsHigher(info->whiskey_data->players_array[i].hand, info->whiskey_data->players_array[looser_index].hand)){
                            looser_index = i;
                        } 
                    }
                    
                    info->whiskey_data->players_array[looser_index].lives--;
                    if(info->whiskey_data->players_array[looser_index].id == info->playerId){
                        info->message.playerLives--;
                    }
                    
                    printf("Player %d has lost 1 live\n Remaining lives: %d\n", looser_index, info->whiskey_data->players_array[looser_index].lives);
                    if(info->whiskey_data->players_array[looser_index].lives <= 0){
                        removePlayer(info->whiskey_data->players_array[looser_index].id, info->whiskey_data);
                        // if(info->whiskey_data->numPlayers == 1){
                        //     getWinner(info->whiskey_data);
                        // }
                        printf("Player was eliminated BYE BYE\n");
                    }
                    pthread_cond_broadcast(&betsReady_condition);
                } 
                pthread_mutex_unlock(&betsReady_mutex);
                printf("UNLOCKED\n");
                pthread_cond_broadcast(&getBets_condition);
            } 
        pthread_mutex_unlock(&bets_mutex);

        pthread_mutex_lock(&betsReady_mutex);
            while (info->whiskey_data->index_playerInTurn < info->whiskey_data->numPlayers){ //Do nothing (wait for ths to be false)
                pthread_cond_wait(&betsReady_condition, &betsReady_mutex);
            }
        pthread_mutex_unlock(&betsReady_mutex);       
        //END FINAL ROUND
        //////////////////////////////////          
    }

    send(info->connection_fd, &info->message, sizeof info->message, 0);

    printf("PLAYER WAS KICKED OUT\n");

    printf("\nENDING THREAD WITH CONNECTION: %d\n", info->connection_fd);

    // Finish the connection
    if (!recvData(info->connection_fd, &info->message, sizeof info->message))
    {
        printf("Finish connection within function\n");
        printf("Client disconnected\n");
    }
    info->message.msg_code = BYE;
    send(info->connection_fd, &info->message, sizeof info->message, 0);
    close(info->connection_fd);

    pthread_exit(NULL);
}