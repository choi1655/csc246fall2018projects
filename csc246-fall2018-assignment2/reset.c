/**
  * @file reset.c
  * @author John Choi
  *
  * Program that resets and initializes the board when called.
  */

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/**
  * Called when there is a condition of failing and terminating the program
  * with a message.
  *
  * @param message of the exception or error
  */
void fail(const char *message) {
  fprintf(stderr, "%s\n", message);
  exit(EXIT_FAILURE);
}

/**
  * Initializes a board and returns the instance of it.
  *
  * @return instance of the board
  */
Board initializeBoard() {
  Board board;
  int counter = 1;
  for (int i = 0; i <= 3; i++) {
    for (int j = 0; j <= 3; j++) {
      board.board[i][j] = counter++;
    }
  }
  board.board[3][3] = -1;
  return board;
}

/**
  * Main entry point of this program that resets and reinitializes the
  * board in the shared memory segment.
  *
  * @return exit condition
  */
int main() {
  key_t id = ftok("/afs/unity.ncsu.edu/users/m/mchoi", 'S');
  int shmID = shmget(id, MESSAGE_LIMIT, 0666 | IPC_CREAT);
  if (shmID == -1) {
    fail("Can't create shared memory");
  }

  Board *boardMemory = (Board *)shmat(shmID, 0, 0);
  if (boardMemory == (Board *)-1) {
    fail("Can't map shared memory segment into address space");
  }

  Board board = initializeBoard();
  *(boardMemory) = board;
  Board signal = {1};
  boardMemory[2] = signal;

  return EXIT_SUCCESS;
}
