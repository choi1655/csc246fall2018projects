/**
  * Client source code that sends and receives messages from user and server.
  *
  * @file client.c
  * @author John Choi
  * @version 09212018
  */
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>

/**
  * Function that prints error messages and quit.
  *
  * @param message to print
  */
static void fail(char const *message) {
  fprintf(stderr, "%s\n", message);
  exit(EXIT_FAILURE);
}

/**
  * Main function that executes the program.
  *
  * @param argc number of arguments
  * @param argv list of arguments
  * @return exit status
  */
int main(int argc, const char *argv[]) {
  if (argc != 2) {
    fail("usage: ./client <command>");
  }

  mqd_t serverQueue = mq_open(SERVER_QUEUE, O_WRONLY);
  mqd_t clientQueue = mq_open(CLIENT_QUEUE, O_RDONLY);
  if (serverQueue == -1 || clientQueue == -1) {
    fail("Cannot open queue");
  }
  char message[MESSAGE_LIMIT + 1];
  strcpy(message, argv[1]);

  char *receivedMessage = (char *)malloc(MESSAGE_LIMIT * sizeof(char));
  int lenServer = mq_send(serverQueue, message, strlen(message), 0);
  if (lenServer < 0) {
    fail("Cannot send message to server");
  }
  int lenClient = mq_receive(clientQueue, receivedMessage, MESSAGE_LIMIT, NULL);
  if (lenClient < 0) {
    fail("Cannot receive message from server");
  }
  fprintf(stdout, "%s\n", receivedMessage);
  free(receivedMessage);
  return EXIT_SUCCESS;
}
