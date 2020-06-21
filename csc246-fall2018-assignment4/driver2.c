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

/** Code for Judy */
void *judy( void *arg ) {
  const char *name = "judy";
  
  sleep( 1 ); // Wait 1 second before trying to enter.
  
  if ( ! enter( name ) )
    return NULL;
  
  sleep( 7 );   // Try to leave after 7 seconds.
  
  leave( name );
  
  return NULL;
}

/** Code for Will */
void *will( void *arg ) {
  const char *name = "will";
  
  sleep( 2 ); // Wait 2 seconds before trying to enter.
  
  if ( ! enter( name ) )
    return NULL;
  
  sleep( 4 );   // Try to leave after 4 seconds.
  
  leave( name );
  
  return NULL;
}

/** Code for Penny */
void *penny( void *arg ) {
  const char *name = "penny";
  
  sleep( 3 ); // Wait 3 seconds before trying to enter.
  
  if ( ! enter( name ) )
    return NULL;
  
  sleep( 3 );   // Try to leave after 3 seconds.
  
  leave( name );
  
  return NULL;
}

/** Code for Maureen */
void *maureen( void *arg ) {
  const char *name = "maureen";
  
  sleep( 4 ); // Wait 4 seconds before trying to enter.
  
  if ( ! enter( name ) )
    return NULL;
  
  sleep( 4 );   // Try to leave after 4 seconds.
  
  leave( name );
  
  return NULL;
}

/** Code for Zachary */
void *zachary( void *arg ) {
  const char *name = "zachary";
  
  sleep( 5 ); // Wait 5 seconds before trying to enter.
  
  if ( ! enter( name ) )
    return NULL;

  // Shouldn't make it to the rest of this function.
  sleep( 1 );   // Try to leave after 1 second.
  
  leave( name );
  
  return NULL;
}

int main( int argc, char *argv[] ) {
  // Initialize our monitor.  Give it a capacity of 2.
  initPairMonitor( 2 );

  // Make a few threads
  pthread_t thread[ 5 ];
  if ( pthread_create( thread + 0, NULL, judy, NULL ) != 0 ||
       pthread_create( thread + 1, NULL, will, NULL ) != 0 ||
       pthread_create( thread + 2, NULL, penny, NULL ) != 0 ||
       pthread_create( thread + 3, NULL, maureen, NULL ) != 0 ||
       pthread_create( thread + 4, NULL, zachary, NULL ) != 0 )
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
