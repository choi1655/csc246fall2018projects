/**
  * Server source code that initializes the board and executes commands sent by
  * the client.
  *
  * @file server.c
  * @author John Choi
  * @author CSC246 TS
  * @version 09212018
  */
#include "common.h"
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

/* Macro for posix */
#define _POSIX_SOURCE

/**
  * Print out an error message and exit.
  *
  * @param message to print
  */
static void fail(char const *message) {
  fprintf(stderr, "%s\n", message);
  exit(1);
}

// Instance of Board for the current state of the gmae.
Board brd;

/**
  * Initializes the 4 by 4 board.
  */
void initBoard() {
  int num = 1;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      brd.board[i][j] = num;
      num++;
    }
  }
  brd.board[3][3] = -1;
}

/**
  * Handles the board command.
  *
  * @return the board
  */
char *boardCommand() {
  char *output = (char *)malloc(MESSAGE_LIMIT * sizeof(char));
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (brd.board[i][j] == -1) {
        strcat(output, " - ");
      } else if (brd.board[i][j] < 10) {
        char temp[3];
        sprintf(temp, " %d ", brd.board[i][j]);
        strcat(output, temp);
      } else {
        char temp[4];
        sprintf(temp, "%d ", brd.board[i][j]);
        strcat(output, temp);
      }
      // if (j != 3) {
      //   strcat(output, " ");
      // }
    }
    // if (i != 3) {
    //   strcat(output, "\n");
    // }
    strcat(output, "\n");
  }
  return output;
}

/**
  * Handles the up command.
  *
  * @return exit status
  */
char *upCommand() {
  bool able = false;
  bool terminate = false; //used to terminate nested for loops
  for (int i = 0; i < 4 && !terminate; i++) {
    for (int j = 0; j < 4; j++) {
      if (brd.board[i - 1][j] == -1) {
        able = true;
        brd.board[i - 1][j] = brd.board[i][j];
        brd.board[i][j] = -1;
        terminate = true;
        break;
      }
    }
  }
  if (able) {
    return "success";
  }
  return "invalid command";
}

/**
  * Handles the left command.
  *
  * @return exit status*
  */
char *leftCommand() {
  bool able = false;
  bool terminate = false; //used to terminate nested for loops
  for (int i = 0; i < 4 && !terminate; i++) {
    for (int j = 0; j < 4; j++) {
      if (brd.board[i][j - 1] == -1) {
        able = true;
        brd.board[i][j - 1] = brd.board[i][j];
        brd.board[i][j] = -1;
        terminate = true;
        break;
      }
    }
  }
  if (able) {
    return "success";
  }
  return "invalid command";
}

/**
  * Handles the right command.
  *
  * @return exit status
  */
char *rightCommand() {
  bool able = false;
  bool terminate = false; //used to terminate nested for loops
  for (int i = 0; i < 4 && !terminate; i++) {
    for (int j = 0; j < 4; j++) {
      if (brd.board[i][j + 1] == -1) {
        able = true;
        brd.board[i][j + 1] = brd.board[i][j];
        brd.board[i][j] = -1;
        terminate = true;
        break;
      }
    }
  }
  if (able) {
    return "success";
  }
  return "invalid command";
}

/**
  * Handles the down command.
  *
  * @return exit status
  */
char *downCommand() {
  bool able = false;
  bool terminate = false; //used to terminate nested for loops
  for (int i = 0; i < 4 && !terminate; i++) {
    for (int j = 0; j < 4; j++) {
      if (brd.board[i + 1][j] == -1) {
        able = true;
        brd.board[i + 1][j] = brd.board[i][j];
        brd.board[i][j] = -1;
        terminate = true;
        break;
      }
    }
  }
  if (able) {
    return "success";
  }
  return "invalid command";
}

/* true if ctrl c is detected */
volatile bool gotSignal = false;

/**
  * Handles the ctrl c input.
  *
  * @param sig signal
  */
void alarmHandler(int sig) {
  fprintf(stdout, "\n%s\n", boardCommand());
}

/**
  * Main function that executes the server.
  *
  * @param argc number of arguments
  * @param argv list of arguments
  * @return exit status
  */
int main(int argc, char *argv[]) {

  // Remove both queues, in case, last time, this program terminated
  // abnormally with some queued messages still queued.
  mq_unlink(SERVER_QUEUE);
  mq_unlink(CLIENT_QUEUE);

  // Prepare structure indicating maximum queue and message sizes.
  struct mq_attr attr;
  attr.mq_flags = 0;
  attr.mq_maxmsg = 1;
  attr.mq_msgsize = MESSAGE_LIMIT;

  // Make both the server and client message queues.
  mqd_t serverQueue = mq_open(SERVER_QUEUE, O_RDONLY | O_CREAT, 0600, &attr);
  mqd_t clientQueue = mq_open(CLIENT_QUEUE, O_WRONLY | O_CREAT, 0600, &attr);
  if (serverQueue == -1 || clientQueue == -1)
    fail("Can't create the needed message queues");

  char message[MESSAGE_LIMIT];
  char *trimmedMessage = (char *)malloc(MESSAGE_LIMIT * sizeof(char));
  char *sendingMessage = (char *)malloc(MESSAGE_LIMIT * sizeof(char));
  int lenServer, lenClient;

  signal(SIGINT, alarmHandler);
  initBoard(); //initializes the board
  // Repeatedly read and process client messages.
  while (true) {
    // ...
    lenServer = mq_receive(serverQueue, message, sizeof(message), NULL);
    if (lenServer >= 0) {
      int i;
      for (i = 0; i < lenServer; i++) {
        *(trimmedMessage + i) = message[i];
      }
      *(trimmedMessage + i) = '\0';
      if (strcmp(trimmedMessage, "board") == 0) {
        strcpy(sendingMessage, boardCommand());
      } else if (strcmp(trimmedMessage, "up") == 0) {
        strcpy(sendingMessage, upCommand());
      } else if (strcmp(trimmedMessage, "down") == 0) {
        strcpy(sendingMessage, downCommand());
      } else if (strcmp(trimmedMessage, "left") == 0) {
        strcpy(sendingMessage, leftCommand());
      } else if (strcmp(trimmedMessage, "right") == 0) {
        strcpy(sendingMessage, rightCommand());
      } else {
        strcpy(sendingMessage, "invalid command");
      }

    } else {
      break;
    }

    lenClient = mq_send(clientQueue, sendingMessage, MESSAGE_LIMIT, 0);
    if (lenClient < 0) {
      fail("Unable to send message");
    }
  }

  // The following code isn't reachable right now, but it will be useful
  // once you add support for SIGINT.

  // Close our two message queues (and delete them).
  mq_close(clientQueue);
  mq_close(serverQueue);

  // Delete the message queues, this is for normal server termination.
  mq_unlink(SERVER_QUEUE);
  mq_unlink(CLIENT_QUEUE);

  free(trimmedMessage);
  free(sendingMessage);

  return EXIT_SUCCESS;
}
