// Sample driver that's simple enough that we can tell what should happen.
// Two people enter, then two people leave.

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

/** Code for Homer */
void *homer( void *arg ) {
  const char *name = "homer";
  
  sleep( 1 ); // Wait 1 second before trying to enter.
  
  enter( name );
  
  sleep( 3 );   // Try to leave it after 3 seconds.
  
  leave( name );
  
  return NULL;
}

/** Code for Bart */
void *bart( void *arg ) {
  const char *name = "bart";
  
  sleep( 2 ); // Wait 2 seconds before trying to enter.
  
  enter( name );
  
  sleep( 2 );   // Try to leave after 2 seconds.
  
  leave( name );
  
  return NULL;
}

int main( int argc, char *argv[] ) {
  // Initialize our monitor.
  initPairMonitor( 1 );

  // Make a few threads
  pthread_t thread[ 2 ];
  if ( pthread_create( thread + 0, NULL, homer, NULL ) != 0 ||
       pthread_create( thread + 1, NULL, bart, NULL ) != 0 )
    fail( "Can't make a thread we need.\n" );

  // Wait until all the threads finish.
  for ( int i = 0; i < sizeof( thread ) / sizeof( thread[ 0 ] ); i++ )
    pthread_join( thread[ i ], NULL );

  // Free any monitor resources.
  destroyPairMonitor();

  return 0;
}
