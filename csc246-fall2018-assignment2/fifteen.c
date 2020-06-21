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

#define CAPACITY 1024

/* Global pointer to the board in memory */
Board *board;

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
void boardCommand() {
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
void upCommand() {
  bool able = false;
  bool terminate = false; //used to terminate nested for loops
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
  if (!able) {
    fprintf(stdout, "invalid command\n");
  }
}

/**
  * Handles the down command.
  */
void downCommand() {
  bool able = false;
  bool terminate = false; //used to terminate nested for loops
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
  if (!able) {
    fprintf(stdout, "invalid command\n");
  }
}

/**
  * Handles the left command.
  */
void leftCommand() {
  bool able = false;
  bool terminate = false; //used to terminate nested for loops
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
  if (!able) {
    fprintf(stdout, "invalid command\n");
  }
}

/**
  * Handles the right command.
  */
void rightCommand() {
  bool able = false;
  bool terminate = false; //used to terminate nested for loops
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
  if (!able) {
    fprintf(stdout, "invalid command\n");
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
  if (argc != 2) {
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

  board = &boardMemory[0];
  if (strcmp(argv[1], "board") == 0) {
    boardCommand();
  } else if (strcmp(argv[1], "up") == 0) {
    upCommand();
  } else if (strcmp(argv[1], "down") == 0) {
    downCommand();
  } else if (strcmp(argv[1], "left") == 0) {
    leftCommand();
  } else if (strcmp(argv[1], "right") == 0) {
    rightCommand();
  } else {
    fail("invalid command");
  }
  return EXIT_SUCCESS;
}
