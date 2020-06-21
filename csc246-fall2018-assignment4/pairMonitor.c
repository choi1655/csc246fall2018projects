#include "pairMonitor.h"

#include <pthread.h>     // For muptex and condtion variables.
#include <stdlib.h>      // For malloc/free/exit
#include <stdio.h>       // For printf (but not using it)
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <semaphore.h>

// Record for a pair of threads who have entered.
// We have one of these for every pair of threads in the room.
typedef struct {
  // Names for the two threads who entered.
  char name[ 2 ][ NAME_MAX + 1 ];
  bool waiting;
  bool leaving;
  // Add other fields that you need.
  // ...
  // bool leaving[2]; //if they are both true, pair can leave
  pthread_cond_t waitingPartner;
  pthread_cond_t leavingPartner;
} Pair;

// True if we're still running.
bool running = true;

// Capacity for the room we're entering.
int cap;

// List of all pairs of threads that have entered.
// An array of Pair structs with a length determined by capacity.
Pair *pairList;

// Time when we first create the monitor.
struct timeval startTime;

static pthread_mutex_t mutex;
// static pthread_mutex_t leaveMutex;

static bool first = true;

void initPairMonitor( int capacity ) {
  // Remember when the program started running.
  gettimeofday( &startTime, NULL );

  // Create and initialize the list of Pair structs.
  cap = capacity;
  pairList = (Pair *) malloc( sizeof( Pair ) * cap );
  for ( int i = 0; i < cap; i++ ) {
    // No threads are part of this pair yet.
    strcpy( pairList[ i ].name[ 0 ], "" );
    strcpy( pairList[ i ].name[ 1 ], "" );
    // pairList[i].leaving[0] = false;
    // pairList[i].leaving[1] = false;
    // pthread_cond_init(&pairList[i].waiting, NULL);
    pairList[i].waiting = false;
    pairList[i].leaving = false;
    pthread_cond_init(&pairList[i].leavingPartner, NULL);
    pthread_cond_init(&pairList[i].waitingPartner, NULL);
  }
}

// Return the current execution time in milliseconds.
static long elapsedTime() {
  struct timeval endTime;
  gettimeofday( &endTime, NULL );

  long delta = 1000L * ( endTime.tv_sec - startTime.tv_sec ) +
    (long) round( ( endTime.tv_usec - startTime.tv_usec ) / 1000.0 );

  return delta;
}

void destroyPairMonitor() {

  // ...

  free( pairList );
}

int findAvailIndex() {
  for (int i = 0; i < cap; i++) {
    if (pairList[i].waiting) {
      return i;
    } else {
      if (strcmp(pairList[i].name[0], "") == 0 && strcmp(pairList[i].name[1], "") == 0) {
        return i;
      }
    }
  }
  return -1;
}

bool enter( const char *name ) {

  if (first) {
    first = false;
    pthread_mutex_lock(&mutex); //enter the monitor
  } else {
    first = true;
  }
  // Find the index of an available Pair on the pairList.
  int idx = findAvailIndex();
  if (!running) {
    pthread_mutex_unlock(&mutex);
    return false;
  }

  // ...
  while (!pairList[idx].waiting) { //first thread will enter
    if (strcmp(pairList[idx].name[0], "") != 0 && strcmp(pairList[idx].name[1], "") != 0) {
      break;
    }
    pairList[idx].waiting = true;
    strcpy(pairList[idx].name[0], name);
    pthread_cond_wait(&pairList[idx].waitingPartner, &mutex);
  }

  while (pairList[idx].waiting) {
    if (strcmp(pairList[idx].name[0], "") != 0 && strcmp(pairList[idx].name[1], "") != 0) {
      break;
    }
    pairList[idx].waiting = false;
    strcpy(pairList[idx].name[1], name);
    pthread_cond_signal(&pairList[idx].waitingPartner);

  // pairList[idx].name[1] = name;

  // pthread_mutex_unlock(&mutex); //prob don't need this

    // The second thread to join the pair can report when the two threads
    // enter.
    long delta = elapsedTime();
    printf( "Enter: %s %s (%ld.%03ld)\n",
            pairList[ idx ].name[ 0 ], pairList[ idx ].name[ 1 ],
            delta / 1000, delta % 1000 );

  // ...
    pthread_mutex_unlock(&mutex);
  }
  return true;
}

int findThread(const char *name) {
  for (int i = 0; i < cap; i++) {
    if (strcmp(pairList[i].name[0], name) == 0 || strcmp(pairList[i].name[1], name) == 0) {
      return i;
    }
  }
  return -1;
}

void leave( const char *name ) {

  // pthread_mutex_lock(&leaveMutex);
  if (first) {
    first = false;
    pthread_mutex_lock(&mutex);
  } else {
    first = true;
  }
  // Find the index of the Pair object we're part of.
  int idx = findThread(name);
  // ...
  while (!pairList[idx].leaving) {
    pairList[idx].leaving = true;
    // pthread_cond_wait(&pairList[idx].leavingPartner, &leaveMutex);
    pthread_cond_wait(&pairList[idx].leavingPartner, &mutex);
  }
  // while (pairList[idx].leaving) {
  //   pthread_cond_signal(&pairList[idx].leavingPartner);
  //
  //   // The second thread to leave can report when the two threads enter.
  //   long delta = elapsedTime();
  //   printf( "Leave: %s %s (%ld.%03ld)\n",
  //           pairList[ idx ].name[ 0 ], pairList[ idx ].name[ 1 ],
  //           delta / 1000, delta % 1000 );
  //
  // // ...
  //   strcpy(pairList[idx].name[0], "");
  //   strcpy(pairList[idx].name[1], "");
  //   // pthread_mutex_unlock(&leaveMutex);
  //   // pthread_mutex_unlock(&leaveMutex);
  //   pthread_mutex_unlock(&mutex);
  //   pthread_mutex_unlock(&mutex);
  //
  //   pairList[idx].leaving = false;
  //   pairList[idx].waiting = false;
  // }
  pthread_cond_signal(&pairList[idx].leavingPartner);

  // The second thread to leave can report when the two threads enter.
  long delta = elapsedTime();
  printf( "Leave: %s %s (%ld.%03ld)\n",
          pairList[ idx ].name[ 0 ], pairList[ idx ].name[ 1 ],
          delta / 1000, delta % 1000 );

// ...
  strcpy(pairList[idx].name[0], "");
  strcpy(pairList[idx].name[1], "");
  // pthread_mutex_unlock(&leaveMutex);
  // pthread_mutex_unlock(&leaveMutex);
  pthread_mutex_unlock(&mutex);
  pthread_mutex_unlock(&mutex);

  /**
    * Adding on 11292018
    */
  // sem_t semaphore;
  // sem_init(&semaphore, 0, 1);
  //lock the semaphore
  // sem_wait(&semaphore);


  pairList[idx].leaving = false;
  pairList[idx].waiting = false;

  // sem_post(&semaphore);
}

void terminate() {
  //
  running = false;
  for (int i = 0; i < cap; i++) {
    pthread_cond_signal(&pairList[i].waitingPartner);
    pthread_cond_signal(&pairList[i].leavingPartner);
  }
}
