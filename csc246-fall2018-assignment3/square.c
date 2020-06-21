#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

// Size of the square we're looking for.
#define SQUARE_WIDTH 6
#define SQUARE_HEIGHT 6

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Print out a usage message, then exit.
static void usage() {
  printf( "usage: square <workers>\n" );
  printf( "       square <workers> report\n" );
  exit( 1 );
}

// True if we're supposed to report what we find.
bool report = false;

// Size of the grid of characters.
int rows, cols;

// Number of rows we've filled in so far.
int rowCount = 0;

// Maximum width of a row.  Makes it easier to allocate the whole
// grid contiguously.
#define MAX_WIDTH 16384

// Type used for a row of the grid.  Makes it easier to declare the
// grid as a pointer.
typedef char Row[ MAX_WIDTH ];

// Grid of letters.
Row *grid;

// Total number of complete squares we've found.
int total = 0;

// Read the grid of characters.
void readGrid() {
  // Read grdi dimensions.
  scanf( "%d%d", &rows, &cols );
  if ( cols > MAX_WIDTH ) {
    fprintf( stderr, "Input grid is too wide.\n" );
    exit( EXIT_FAILURE );
  }

  // Make space to store the grid as a big, contiguous array.
  grid = (Row *) malloc( rows * sizeof( Row ) );

  // Read each row of the grid as a string, then copy everything
  // but the null terminator into the grid array.
  char buffer[ MAX_WIDTH + 1 ];
  while ( rowCount < rows ) {
    scanf( "%s", buffer );

    memcpy( grid[ rowCount ], buffer, cols );
    rowCount += 1;
  }
}

/* keeps in track of the current row */
int volatile currentRow = 0;
/* number of workers */
int numWorker;
/* number of current running worker */
int volatile currentWorker = 0;

sem_t lock;
sem_t editingTotal;
sem_t searching;

/**
  * Checks individual 6 by 6 squares to see if it is a valid square.
  *
  * @param squares - individual squares to check
  * @return true if this square is valid
  */
bool checkSquare(char squares[6][6]) {
  char *knownLetters = (char *)malloc(26 * sizeof(char));
  for (int i = 0; i < 26; i++) {
    *(knownLetters + i) = '*';
  }
  int counter = 0;
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 6; j++) {
      bool exists = false;
      for (int k = 0; k <= counter; k++) {
        if (squares[i][j] == *(knownLetters + k)) { //if letter is found
          exists = true;
        }
      }
      if (!exists) {
        *(knownLetters + counter) = squares[i][j];
        counter++;
      }
    }
  }
  free(knownLetters);
  if (counter != 26) {
    return false;
  }
  return true;
}

/** Start routine for each worker. */
void *workerRoutine( void *arg ) {
  sem_wait(&searching);
  // ...
  char square[6][6];
  int lowerBoundRow = currentWorker + (currentRow * numWorker);
  int upperBoundRow = lowerBoundRow + 5;

  int lowerBoundCol = 0;
  int upperBoundCol = lowerBoundCol + 5;
  int currentLocationA = 0; //used for square
  int currentLocationB = 0; //used for square

  while (upperBoundRow < rows) { //one process increments - scans vertically
    while (upperBoundCol < cols) { //scans horizontally
      for (int i = lowerBoundRow; i <= upperBoundRow; i++) { //for row
        for (int j = lowerBoundCol; j <= upperBoundCol; j++) { // for col
          square[currentLocationA][currentLocationB++] = grid[i][j];
        }

        currentLocationA++;
        currentLocationB = 0;
      }
      currentLocationA = 0;

      if (checkSquare(square)) {
        if (report) {
          fprintf(stdout, "%d %d\n", lowerBoundRow, lowerBoundCol);
        }
        sem_wait(&editingTotal);
        total++;
        sem_post(&editingTotal);
      }
      lowerBoundCol++;
      upperBoundCol++;
    }
    currentRow++;
    lowerBoundRow = currentWorker + (currentRow * numWorker);
    upperBoundRow = lowerBoundRow + 5;
    lowerBoundCol = 0;
    upperBoundCol = lowerBoundCol + 5;
  }
  currentWorker++;
  sem_post(&searching);
  return NULL;
}

int main( int argc, char *argv[] ) {
  int workers = 4;

  // Parse command-line arguments.
  if ( argc < 2 || argc > 3 )
    usage();

  if ( sscanf( argv[ 1 ], "%d", &workers ) != 1 ||
       workers < 1 )
    usage();

  // If there's a second argument, it better be "report"
  if ( argc == 3 ) {
    if ( strcmp( argv[ 2 ], "report" ) != 0 )
      usage();
    report = true;
  }
  numWorker = workers;
  sem_init(&lock, 0, 1);
  sem_init(&editingTotal, 0, 1);
  sem_init(&searching, 0, 1);
  // Make each of the workers.
  pthread_t worker[ workers ];
  for ( int i = 0; i < workers; i++ ) {
    // ...
    if (pthread_create(&worker[i], NULL, workerRoutine, NULL) != 0) {
      fail("Can't create thread");
    }
  }
  // Then, start getting work for them to do.
  readGrid();
  // Wait until all the workers finish.
  for ( int i = 0; i < workers; i++ )
    // ...
    pthread_join(worker[i], NULL);
  // Report the total and release the semaphores.
  printf( "Squares: %d\n", total );

  return EXIT_SUCCESS;
}
