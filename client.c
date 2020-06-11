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
// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>
// Custom libraries
#include "codes.h"
#include "sockets.h"

#define BUFFER_SIZE 1024

///// FUNCTION DECLARATIONS
void usage(char * program);
void communicationLoop(int connection_fd);

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int connection_fd;

    printf("\n=== CLIENT PROGRAM ===\n");

    // Check the correct arguments
    if (argc != 3)
    {
        usage(argv[0]);
    }

    // Start the server
    connection_fd = connectSocket(argv[1], argv[2]);
	

    // Establish the communication
    communicationLoop(connection_fd);

    // Close the socket
    close(connection_fd);

    return 0;
}

///// FUNCTION DEFINITIONS

/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char * program)
{
    printf("Usage:\n");
    printf("\t%s {server_address} {port_number}\n", program);
    exit(EXIT_FAILURE);
}


// Do the actual receiving and sending of data
void communicationLoop(int connection_fd)
{
    message_t message; //message with the information that will be updated between server and client
    int round = 0;
    int start_game = 1;
    // Handshake
    message.msg_code = PLAY;
    send(connection_fd, &message, sizeof message, 0);

    //Check reply, receive AMOUNT
    if (!recvData(connection_fd, &message, sizeof message)) //The final results are received from the server
    {
        printf("Connection refused by the server");
        return;
    }

    if(message.msg_code == FULL){
        printf("This game is full. The server rejected the client.\n");
        return;
    }else if (message.msg_code != AMOUNT)
    {
        printf("Invalid server\n");
        return;
    }

    // BET AMMOUNT 
    message.playerAmount = 100;

    send(connection_fd, &message, sizeof message, 0);

    //Check if the player is ready to start
    recvData(connection_fd, &message, sizeof message);

    while(start_game){ 
        printf("Press 2 to start the game\n");
        fflush( stdout );
        scanf("%d", &start_game);
        if(start_game == 2){
            //printf("note='%d'\n", start_game);
            start_game = 0;
        }
    }

    printf("------------WAITNING FOR THE REST OF THE PLAYERS TO START, GET READY -------------\n");

    message.theStatus = LOBBY;
    //SEND START
    send(connection_fd, &message, sizeof message, 0);

    // Get the OK to start the game loop, receive STARTs
    recvData(connection_fd, &message, sizeof message);

    while(message.playerAmount >= 2) //While the player has enough money to bet
    {
        round++;

        printf("\n|||||||||||||||ROUND %d|||||||||||||||\n", round);

        printf("\n/////ASKING FOR THE BET/////\n");
        printf("YOU HAVE %d LIVES\n", message.playerLives);
        while(1){
            send(connection_fd, &message, sizeof message, 0);
            printf("Waiting other players to finish turn\n");

            //Check if the player is ready to start the round
            recvData(connection_fd, &message, sizeof message);

            printf("Finished turn\n");

            
            
            //Send ready for round
            send(connection_fd, &message, sizeof message, 0);

            // Get the signal to start the game loop, receive STARTs
            recvData(connection_fd, &message, sizeof message);

            int playerOption =  0;

            printf("TURN %d\n", message.turn);
            printf("TABLE HAND:\n");
            if(message.turn == 1){
                printf("The hand is hidden\n");
            }else{
                printHand(message.whiskeyHand);
            }
            printf("YOUR HAND:\n");
            printHand(message.playerHand);
            
            //If it is the first turn the Viuda opens and player can only change the full hand or not
            if(message.turn == 1){
                while((playerOption != 1) && (playerOption != 2) && (playerOption != 3)){
                    printf("\nChoose one of the options:\n1: Knock\n2: Change all cards\n3: Pass\n");
                    scanf("%d", &playerOption);
                    switch(playerOption){
                        case 1:
                            message.playerStatus = KNOCK;  //The player has a game and only one more round is left
                            break;
                        case 2:
                            message.playerStatus = CHANGE_ALL; //Change the full hand
                            changeAllCards(&message.whiskeyHand, &message.playerHand);
                            break;
                        case 3:
                            message.playerStatus = PASS; // The client those nothing
                            break;
                        default:
                            printf("You should press either 1, 2, or 3\n");
                    }
                }
            }else{ //The first turn passed
                while((playerOption != 1) && (playerOption != 2) && (playerOption != 3)){
                    printf("\nChoose one of the options:\n1: Knock\n2: Change one card\n3: Change all cards\n");
                    scanf("%d", &playerOption);
                    switch(playerOption){
                        case 1:
                            message.playerStatus = KNOCK; //The player has a game and only one more round is left
                            break;
                        case 2:
                            message.playerStatus = CHANGE_ONE; //Change only one card
                            changeOneCard(&message.whiskeyHand, &message.playerHand);
                            break;
                        case 3:
                            message.playerStatus = CHANGE_ALL; //Change the full hand
                            changeAllCards(&message.whiskeyHand, &message.playerHand);
                            break;
                        default:
                            printf("You should press either 1, 2, or 3\n");
                    }
                }
            }

            send(connection_fd, &message, sizeof message, 0);

        }

        //Receives the total hand accumulated by the player
        if (!recvData(connection_fd, &message, sizeof message))
        {
            return;
        }

        
        //Receive results made by the dealer
        if (!recvData(connection_fd, &message, sizeof message)) //The final results are received from the server
        {
            return;
        }

        if(message.playerAmount < 2) { //Check if the player can keep playing, if cannot, disconnect.
            printf("You don't have enough money to keep playing, goodbye!\n");
            return;
        } else {
            //Reset status of dealer and player for next round
            message.playerStatus = START;
            
        }
    }

    // Finish the communication
    message.msg_code = BYE;
    send(connection_fd, &message, sizeof message, 0);
    recvData(connection_fd, &message, sizeof message);
}