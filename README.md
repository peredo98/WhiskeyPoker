# WhiskeyPoker
---
##### Team:
Emiliano Peredo A01422326
Raziel Nicolás Martínez Castillo A01410695
Camila Rovirosa Ochoa A01024192

---
### Whiskey poker game in C
In this assignment we will implement a variation of the game Whiskey Poker, called “Viuda Negra” using different programming topics in order to be able to connect different computers to play the game.
The topics that will be necessary to complete this assignment are:  Dynamic memory, Pointers, Process creation and threads.

### Rules of the Game
The rules of the game
The game of Whiskey Poker is composed of a set of different rules from which we had taken the next rules for this project: 
```
  1. The game can be played from 2-8 players using a standard 52-card pack without jokers.
  2. The objective is to collect the best 5-card poker hand, by exchanging cards with a spare hand dealt to the table and be the last player alive at the end of the game.
  3. Before the game each player  gets 3 lives (chips). Each round he/she can lose a life or stay with the same. If one player loses all his/her lives he/she is out of the game.
  4. At the beginning of the game each player gets 5 random cards and 5 cards are faced-down in the center of the table.
  5. The first player in turn can decide if he wants to change all his/her cards with the ones that are in the table. If he does, he has to take the 5 cards and put his previous cards face-up in the center of the table. If he doesn’t, the next player in turn takes the same decision. This is done until one player decides to change his cards. If no one changes his cards then every player shows his/her cards and the winner is chosen based on their hands.
  6. If a player changed his/her cards then the game keeps going with the cards in the table faced-up. The next player can change his 5 cards or exchange one with the ones in the table putting one of his cards face-up in his turn and so on with the next players.
  7. The game keeps going until one player decides to “Knock” in his turn. The player that stopped cannot make any exchange in that turn. When this happens it means that it is the last round and every player after him has one last turn to exchange cards until they reach the player who stopped.
  8. When this finish every player show his cards. The player who has the worst hand lose one life (chip).  
  9.The last player alive (the one that has 1 or more lives), gets the bets of all the players.
  10. After the knok the game ends
```
#### List of how you will use the topics seen in class
A quick overview of how we will use them as seen in the solution:
  - Dynamic memory: We will need to create an array of structs to store the data of each player (cards, bets, viuda, etc.). As there can be a different amount of players each game it will be necessary to have a dynamic array at the beginning of the game.
  - Pointers: We will use this by passing a structure as reference between functions in the server to make calculations and are also necessary to read and send between a server and client.
  - Process creation: This is also necessary when creating concurrent clients.
  - Threads: We will use them in the server to keep track of the clients and make them do operations in the order we want depending on their turn.
  
  #### The program running:
  We decided not to create a "dummy client" because we risk having the player play forever in the same round since it may take a lot of time before a dummy client decides to knock
  -First clone this repository  using  " git clone " command
  -It is preferable that this program is run in linuc or the Ubuntu wsl
  -Once cloned go to the direction where you save the repository and open it
  -To compile this program use the command "make" in the terminal
  -You will need at least thre differnt teriminals, one for the server, and at least two one per client or player you want to use.
    You can have a minimum of two players and a maximun of 8
    
  -First run the server with this command "./server 8400 "
  -Then per client run this command "./client localhost 8400 "
    You will se something like this:
    ![alt text](https://github.com/peredo98/WhiskeyPoker/blob/master/tutorial.png)
    Follow the instrucctions and have fun!
