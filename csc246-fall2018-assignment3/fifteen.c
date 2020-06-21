/**
  * @file fifteen.c
  * @author John Choi
  *
  * Program that modifies the board and handles commands from
  * the user.
  */

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>

#define CAPACITY 1024

/* Global pointer to the board in memory */
//Board *board;
/* Global semaphore */
#ifndef UNSAFE
sem_t *lock;
#endif

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
  * Handles the board command.
  */
void boardCommand(Board *board) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (board->board[i][j] != -1) {
        if (board->board[i][j] < 10) {
          fprintf(stdout, " %d ", board->board[i][j]);
        } else {
          fprintf(stdout, "%d ", board->board[i][j]);
        }
      } else {
        fprintf(stdout, " %c ", '-');
      }
    }
    fprintf(stdout, "\n");
  }
}

/**
  * Handles the up command.
  */
bool upCommand(Board *board) {
  bool able = false;
  bool terminate = false; //used to terminate nested for loops
#ifndef UNSAFE
  sem_wait(lock);
#endif
  for (int i = 1; i < 4 && !terminate; i++) {
    for (int j = 0; j < 4; j++) {
      if (board->board[i - 1][j] == -1) {
        able = true;
        board->board[i - 1][j] = board->board[i][j];
        board->board[i][j] = -1;
        terminate = true;
        break;
      }
    }
  }
#ifndef UNSAFE
  sem_post(lock);
#endif
  if (!able) {
    fprintf(stdout, "invalid command\n");
  }
  return able;
}

/**
  * Handles the down command.
  */
bool downCommand(Board *board) {
  bool able = false;
  bool terminate = false; //used to terminate nested for loops
#ifndef UNSAFE
  sem_wait(lock);
#endif
  for (int i = 0; i < 4 && !terminate; i++) {
    for (int j = 0; j < 4; j++) {
      if (board->board[i + 1][j] == -1) {
        able = true;
        board->board[i + 1][j] = board->board[i][j];
        board->board[i][j] = -1;
        terminate = true;
        break;
      }
    }
  }
#ifndef UNSAFE
  sem_post(lock);
#endif
  if (!able) {
    fprintf(stdout, "invalid command\n");
  }
  return able;
}

/**
  * Handles the left command.
  */
bool leftCommand(Board *board) {
  bool able = false;
  bool terminate = false; //used to terminate nested for loops
#ifndef UNSAFE
  sem_wait(lock);
#endif
  for (int i = 0; i < 4 && !terminate; i++) {
    for (int j = 0; j < 4; j++) {
      if (board->board[i][j - 1] == -1) {
        able = true;
        board->board[i][j - 1] = board->board[i][j];
        board->board[i][j] = -1;
        terminate = true;
        break;
      }
    }
  }
#ifndef UNSAFE
  sem_post(lock);
#endif
  if (!able) {
    fprintf(stdout, "invalid command\n");
  }
  return able;
}

/**
  * Handles the right command.
  */
bool rightCommand(Board *board) {
  bool able = false;
  bool terminate = false; //used to terminate nested for loops
#ifndef UNSAFE
  sem_wait(lock);
#endif
  for (int i = 0; i < 4 && !terminate; i++) {
    for (int j = 0; j < 4; j++) {
      if (board->board[i][j + 1] == -1) {
        able = true;
        board->board[i][j + 1] = board->board[i][j];
        board->board[i][j] = -1;
        terminate = true;
        break;
      }
    }
  }
#ifndef UNSAFE
  sem_post(lock);
#endif
  if (!able) {
    fprintf(stdout, "invalid command\n");
  }
  return able;
}

// Test interface, for quickly making moves on the board.
void test( Board *brd, int n ) {
  for ( int i = 0; i < n; i++ ) {
    rightCommand( brd );
    downCommand( brd );
    leftCommand( brd );
    upCommand( brd ); 
  }
}

/**
  * Entry point of this program that executes the program.
  * Exits the program if invalid command was inputted.
  *
  * @param argc number of arguments passed in
  * @param argv command-line arguments
  * @return exit condition
  */
int main(int argc, char** argv) {
  if (argc != 2 && argc != 3) {
    fail("invalid command");
  }

  key_t id = ftok("/afs/unity.ncsu.edu/users/m/mchoi", 'S');
  bool memoryExists = false;
  int shmID = shmget(id, CAPACITY, 0666 | IPC_CREAT);
  if (shmID == -1) {
    fail("Can't create shared memory");
  }
  Board *boardMemory = (Board *)shmat(shmID, 0, 0);
  if (boardMemory == (Board *)-1) {
    fail("Can't map shared memory segment into address space");
  }
  if (boardMemory[2].signal == 1) { //check if the last digit in memory is 1
    memoryExists = true;
  }
  if (!memoryExists) {
    fail("Board is not initialized. Run ./reset first");
  }
#ifndef UNSAFE
  lock = sem_open("/mchoi-board-lock", 0);
  if (lock == SEM_FAILED) {
    fail("Cannot make lock semaphore");
  }
#endif
  //board = &boardMemory[0];
  if (strcmp(argv[1], "board") == 0) {
    boardCommand(&boardMemory[0]);
  } else if (strcmp(argv[1], "up") == 0) {
    upCommand(&boardMemory[0]);
  } else if (strcmp(argv[1], "down") == 0) {
    downCommand(&boardMemory[0]);
  } else if (strcmp(argv[1], "left") == 0) {
    leftCommand(&boardMemory[0]);
  } else if (strcmp(argv[1], "right") == 0) {
    rightCommand(&boardMemory[0]);
  } else if (strcmp(argv[1], "test") == 0) {
    int n = atoi(argv[2]);
    test(&boardMemory[0], n);
  } else {
    fail("invalid command");
  }
#ifndef UNSAFE
  sem_close(lock);
#endif
  return EXIT_SUCCESS;
}
