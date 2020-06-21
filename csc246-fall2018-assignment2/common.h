/**
  * Common header file that defines the struct with board and commonly used
  * magic numbers.
  *
  * @file common.h
  * @author CSC246 TS
  * @author John Choi
  * @version 10072018
  */
#ifndef _COMMON_H_
#define _COMMON_H_

// Name for the queue of messages going to the server.
#define SERVER_QUEUE "/dbsturgi-server-queue"

// Name for the queue of messages going to the current client.
#define CLIENT_QUEUE "/dbsturgi-client-queue"

// Maximum length for a message in the queue
// (Long enough to return a copy of the whole board)
#define MESSAGE_LIMIT 1024

// Height of the board.
#define BOARD_ROWS 4

// Width of the board.
#define BOARD_COLS 4

// Representation for the board and votes for each candidate.
typedef struct {
  // Define your own representation.
  // ...
  int signal;
  int board[5][5];
} Board;

#endif
