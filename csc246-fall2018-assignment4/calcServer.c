/**
  * @file calcServer.c
  * @author John Choi mchoi
  *
  * CSC246 Fall 2018 Assignment 4 Part 3
  */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <semaphore.h>

/** Port number used by mchoi server */
#define PORT_NUMBER "26262"
/** Second number used by mchoi server */
//#define PORT_NUMBER "26263"

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

/** 26 possible letters */
int variables[26];
/** 26 semaphores each dedicated to a letter */
sem_t locks[26];
sem_t commandLock;

/**
  * Initializes 26 semaphores
  */
void initializeSemaphores() {
  for (int i = 0; i < 26; i++) {
    sem_init(&locks[i], 0, 1);
  }
  sem_init(&commandLock, 0, 1);
}

/**
  * Destroys all 26 semaphores
  */
void destroySemaphores() {
  for (int i = 0; i < 26; i++) {
    sem_destroy(&locks[i]);
  }
  sem_destroy(&commandLock);
}

/** handle a client connection, close it when we're done. */
void *handleClient( void *arg ) {
  initializeSemaphores();
  // Here's a nice trick, wrap a C standard IO FILE around the
  // socket, so we can communicate the same way we would read/write
  // a file.
  int *sockP = (int *)arg;
  int sock = *sockP;
  FILE *fp = fdopen( sock, "a+" );

  // Prompt the user for a command.
  fprintf( fp, "cmd> " );

  char cmd[ 11 ];
  sem_wait(&commandLock);
  while ( fscanf( fp, "%10s", cmd ) == 1 && strcmp( cmd, "quit" ) != 0 ) {
    // sleep(2); //NOTE used to test for racing condition
    sem_post(&commandLock);
    // Just echo the command back to the client.
    // fprintf( fp, "%s\n", cmd );
    //Read in the command
    if (strcmp(cmd, "set") == 0) {
      char temp = -1;
      char *value = (char *)malloc(20);
      if (fscanf(fp, " %[a-z]%19s%*[^\n]\n", &temp, value) != 2) {
        fprintf(fp, "Invalid input. Use Ctrl+C to terminate\n");
        fail("Did not read 2 values");
      }
      temp -= 97;
      int idx = temp;

      /** if the second variable is a letter */
      if (strlen(value) == 1 && (value[0] >= 'a' && value[0] <= 'z')) {
        int secondIdx = value[0] - 97;
        // lock first and second semaphores
        sem_wait(&locks[idx]);
        sem_wait(&locks[secondIdx]);

        variables[idx] = variables[secondIdx];

        // unlock second and first semaphores
        sem_post(&locks[secondIdx]);
        sem_post(&locks[idx]);
      } else {
        //lock semaphore
        sem_wait(&locks[idx]);
        variables[idx] = atoi(value);

        //unlock semaphore
        sem_post(&locks[idx]);
      }
      free(value);
    } else if (strcmp(cmd, "print") == 0) {
      char temp = -1;
      if (fscanf(fp, " %[a-z]%*[^\n]\n", &temp) != 1) {
        fprintf(fp, "Invalid input. Use Ctrl+C to terminate\n");
        fail("Did not match 1");
      }
      temp -= 97;
      int idx = temp;
      //lock semaphore
      sem_wait(&locks[idx]);
      fprintf(fp, "%d\n", variables[idx]);
      //unlock sempahore
      sem_post(&locks[idx]);
    } else if (strcmp(cmd, "add") == 0 || strcmp(cmd, "subtract") == 0 || strcmp(cmd, "multiply") == 0 || strcmp(cmd, "divide") == 0) {
      char v1;
      char *tempv2 = (char *)malloc(11);
      if (fscanf(fp, " %[a-z] ", &v1) != 1) {
        fprintf(fp, "Invalid input. Use Ctrl+C to terminate\n");
        fail("Did not match a");
      }
      if (fscanf(fp, "%s%*[^\n]\n", tempv2) != 1) {
        fprintf(fp, "Invalid input. Use Ctrl+C to terminate\n");
        fail("Did not match b");
      }
      bool isNumber = false; //indicates if third argument is a number
      char v2 = tempv2[0]; //initially set v2 to first char of string
      if ((tempv2[0] >= '0' && tempv2[0] <= '9')) { //if first char is a number
        v2 = atoi(tempv2); //it means it's number
        isNumber = true;
      }
      v1 -= 97;
      free(tempv2);
      int a, b;
      if (isNumber) {
        sem_wait(&locks[(int) v1]);
        a = variables[(int) v1];
        sem_post(&locks[(int) v1]);
        b = v2;
      } else {
        v2 -= 97;
        //lock v1 semaphore
        sem_wait(&locks[(int) v1]);
        //lock v2 semaphore
        sem_wait(&locks[(int) v2]);
        a = variables[(int) v1];
        b = variables[(int) v2];
      }
      if (strcmp(cmd, "add") == 0) {
        a += b;
        variables[(int) v1] = a;
        // fprintf(fp, "%d\n", a);
      } else if (strcmp(cmd, "subtract") == 0) {
        a -= b;
        variables[(int) v1] = a;
        // fprintf(fp, "%d\n", a);
      } else if (strcmp(cmd, "multiply") == 0) {
        a *= b;
        variables[(int) v1] = a;
        // fprintf(fp, "%d\n", a);
      } else /*if (strcmp(cmd, "divide") == 0)*/ {
        if (b == 0) {
          fprintf(fp, "Invalid command\n");
        } else {
          a /= b;
          variables[(int) v1] = a;
          // fprintf(fp, "%d\n", a);
        }
      }
      //unlock v2 semaphore
      sem_post(&locks[(int) v2]);
      //unlock v1 semaphore
      sem_post(&locks[(int) v1]);
    } else {
      fprintf(fp, "Invalid command\n");
    }

    // Prompt the user for the next command.
    fprintf( fp, "cmd> " );
    fflush( fp );
    sem_wait(&commandLock);
  }

  sem_wait(&commandLock);
  // Close the connection with this client.
  fclose( fp );
  destroySemaphores();
  return NULL;
}

/**
  * Driver function for server.
  *
  * @return EXIT_SUCCESS if exits successfully
  */
int main() {
  // Prepare a description of server address criteria.
  struct addrinfo addrCriteria;
  memset(&addrCriteria, 0, sizeof(addrCriteria));
  addrCriteria.ai_family = AF_INET;
  addrCriteria.ai_flags = AI_PASSIVE;
  addrCriteria.ai_socktype = SOCK_STREAM;
  addrCriteria.ai_protocol = IPPROTO_TCP;

  // Lookup a list of matching addresses
  struct addrinfo *servAddr;
  if ( getaddrinfo( NULL, PORT_NUMBER, &addrCriteria, &servAddr) )
    fail( "Can't get address info" );

  // Try to just use the first one.
  if ( servAddr == NULL )
    fail( "Can't get address" );

  // Create a TCP socket
  int servSock = socket( servAddr->ai_family, servAddr->ai_socktype,
                         servAddr->ai_protocol);
  if ( servSock < 0 )
    fail( "Can't create socket" );

  // Hopefully this will stop the occasional "Can't bind socket"
  if (setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, (int []){1},
                 sizeof(int)) == -1)
    fail( "Can't set socket options" );

  // Bind to the local address
  if ( bind(servSock, servAddr->ai_addr, servAddr->ai_addrlen) != 0 )
    fail( "Can't bind socket" );

  // Tell the socket to listen for incoming connections.
  if ( listen( servSock, 5 ) != 0 )
    fail( "Can't listen on socket" );

  // Free address list allocated by getaddrinfo()
  freeaddrinfo(servAddr);

  // Fields for accepting a client connection.
  struct sockaddr_storage clntAddr; // Client address
  socklen_t clntAddrLen = sizeof(clntAddr);

  // initializeSemaphores();
  while ( true ) {
    // Accept a client connection.
    int sock = accept( servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);

    // Interact with this client.
    pthread_t thread;
    pthread_create(&thread, NULL, handleClient, &sock);
    // handleClient( sock );
    pthread_detach(thread);
  }
  // destroySemaphores();
  // Stop accepting client connections (never reached).
  close( servSock );

  return 0;
}
