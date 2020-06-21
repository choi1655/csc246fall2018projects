// Elapsed Real Time for input-4.txt: real	0m29.789s

#include <stdio.h>
#include <stdbool.h>
#include <cuda_runtime.h>

// Size of the square we're looking for.
#define SQUARE_WIDTH 6
#define SQUARE_HEIGHT 6

// Maximum width of a row.  Makes it easier to allocate the whole
// grid contiguously.
#define MAX_WIDTH 16384

// Type used for a row of the grid.  Makes it easier to declare the
// grid as a pointer.
typedef char Row[ MAX_WIDTH ];

// Kernel, run by each thread to count complete squares in parallel.
__global__ void countSquares( int rows, int cols, bool report, Row *grid, int *output) {
  // Unique index for this worker.
  int r0 = blockDim.x * blockIdx.x + threadIdx.x;

  // Make sure I actually have something to work on.
  if ( r0 + SQUARE_HEIGHT - 1 < rows ) {
    int total = 0;
    //TODO logic goes here
    int startIdxCols = 0; //this is actually column
    int endIdxCols = startIdxCols + 6;
    int startIdxRows = r0; //so this is actually row
    int endIdxRows = startIdxRows + 6;
    int colidx = 0;
    int rowidx = 0;
    char square[6][6];
    while (endIdxCols <= cols) {
      //fill in square 2d array
      for (int i = startIdxRows; i < endIdxRows; i++) {
        for (int j = startIdxCols; j < endIdxCols; j++) {
          square[rowidx][colidx] = grid[i][j];
          // printf("%c %d %d\n", grid[i][j], i, j);
          // printf("%c\n", square[rowidx][colidx]);
          colidx++;
        }
        rowidx++;
        colidx = 0;
      }
      rowidx = 0;

      //at this point square is made
      //so check if it's valid square
      bool isValid = false;
      char *knownLetters = (char *)malloc(26 * sizeof(char));
      for (int i = 0; i < 26; i++) {
        *(knownLetters + i) = '*';
      }
      int counter = 0;
      for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
          bool exists = false;
          for (int k = 0; k <= counter; k++) {
            if (square[i][j] == *(knownLetters + k)) { //if letter is found
              exists = true;
            }
          }
          if (!exists) {
            *(knownLetters + counter) = square[i][j];
            counter++;
          }
        }
      }
      free(knownLetters);
      if (counter == 26) {
        isValid = true;
      }
      //End of boolean function

      if (isValid) {
        total++;
        if (report) {
          printf("%d %d\n", startIdxRows, startIdxCols);
        }
      }
      endIdxCols++;
      startIdxCols++;
    } //end of while loop

    *(output + r0) = total; //save the total number of squares to the unique index
    // printf("Total squares is %d with thread ID %d\n", total, r0);
  } //end of if statement
}

// Size of the grid of characters.
int rows, cols;

// Grid of letters.
Row *grid;

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
  int rowCount = 0;
  char buffer[ MAX_WIDTH + 1 ];
  while ( rowCount < rows ) {
    scanf( "%s", buffer );
    memcpy( grid[ rowCount++ ], buffer, cols );
  }
}

// General function to report a failure and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Print out a usage message, then exit.
static void usage() {
  printf( "usage: square [report]\n" );
  exit( 1 );
}

int main( int argc, char *argv[] ) {
  // If there's an argument, it better be "report"
  bool report = false;
  if ( argc == 2 ) {
    if ( strcmp( argv[ 1 ], "report" ) != 0 )
      usage();
    report = true;
  }

  // squareFound = false;
  readGrid();

  /** Array used to hold each squares found in each threads */
  // int *reportedSquares = (int *)malloc(rows * sizeof(int));

  // TODO Need to add code to allocate memory on the device and copy the grid
  // over.
  Row *rowGrid = NULL;
  cudaMalloc((void **)&rowGrid, rows * sizeof(Row)); //allocate memory for 2 arrays
  cudaMemcpy(rowGrid, grid, rows * sizeof(Row), cudaMemcpyHostToDevice);

  // Block and grid dimensions.
  int threadsPerBlock = 250;
  // Round up.
  int blocksPerGrid = ( rows + threadsPerBlock - 1 ) / threadsPerBlock;

  // int *output = (int *)malloc(rows * sizeof(int));
  int *output = NULL;
  cudaMalloc((void **)&output, rows * sizeof(int));
  cudaMemset(output, 0x00, rows * sizeof(int));


  // printf("\n\n");

  // Run our kernel on these block/grid dimensions
  countSquares<<<blocksPerGrid, threadsPerBlock>>>( rows, cols, report, rowGrid, output);
  if ( cudaGetLastError() != cudaSuccess )
    fail( "Failure in CUDA kernel execution." );

  // TODO Need to add code to copy the results list back to the host and
  // add them up.
  // int *squareNums = NULL;
  int *mySquareNums = (int *)malloc(rows * sizeof(int));
  // cudaMalloc((void **)&squareNums, rows * sizeof(int));
  cudaMemcpy(mySquareNums, output, rows * sizeof(int), cudaMemcpyDeviceToHost);

  // for (int i = 0; i < rows; i++) {
  //   *(output + i) = 0;
  //   mySquareNums[i] = 0;
  // }
  int total = 0;
  for (int i = 0; i < rows; i++) {
    total += mySquareNums[i];
  }
  printf( "Squares: %d\n", total );

  cudaFree(rowGrid);
  cudaFree(output);
  free(mySquareNums);
  // Free memory on the device and the host.
  free( grid );

  cudaDeviceReset(); //reset the device

  return 0;
}
