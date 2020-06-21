// Another simple driver that uses a room of capacity 2 and tests the terminate()
// function.

#include <stdio.h>
#include <pthread.h>   // for pthreads
#include <stdlib.h>    // for exit
#include <unistd.h>    // for sleep/usleep

#include "pairMonitor.h"

// General-purpose fail function.  Print a message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Behavior profile for one of our threads, their name, how long the wait before
// entering and how long they wait before trying to leave.
typedef struct {
  const char *name;
  int outTime;
  int inTime;
} Profile;

/** Code for all of our threads, each one with its own name. */
void *person( void *arg ) {
  // Get my profile from the thread argument.
  Profile *me = (Profile *) arg;

  // Keep entering and leaving according to my schedule.  Stop after terminate()
  // is called.
  while ( true ) {
    // Wait a little while.
    usleep( me->outTime );

    // Try to enter.
    if ( ! enter( me->name ) )
      return NULL;
  
    // Wait a little while.
    usleep( me->inTime );

    // Leave (when our partner is ready).
    leave( me->name );
  }
  
  return NULL;
}

int main( int argc, char *argv[] ) {
  // Initialize our monitor.  Give it a capacity of 3.
  initPairMonitor( 3 );

  // Make a some threads that will repeatedly enter then leave.
  pthread_t thread[ 10 ];
  Profile args[ 10 ] =
    {
     { "Madeleine", 300, 550 },
     { "Elouise", 400, 700 },
     { "Sharon", 500, 850 },
     { "Trish", 600, 450 },
     { "Mohamed", 700, 650 },
     { "Jessica", 800, 500 },
     { "Beverly", 900, 800 },
     { "Eleanore", 1000, 750 },
     { "Renate", 1100, 400 },
     { "Judy", 1200, 600 },
  };

  for ( int i = 0; i < sizeof( thread ) / sizeof( thread[ 0 ] ); i++ )
    if ( pthread_create( thread + i, NULL, person, args + i ) != 0 )
      fail( "Can't make a thread we need.\n" );

  // Terminate after 10 seconds.
  sleep( 10 );
  terminate();

  // Wait until all the threads finish.
  for ( int i = 0; i < sizeof( thread ) / sizeof( thread[ 0 ] ); i++ )
    pthread_join( thread[ i ], NULL );

  // Free any monitor resources.
  destroyPairMonitor();

  return 0;
}
