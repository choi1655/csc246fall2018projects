/**
  * CSC246 Assignment 1 problem 3.
  *
  * @author John Choi
  * @version 09202018
  */
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

// Size of the square we're looking for.
#define SQUARE_WIDTH 6
#define SQUARE_HEIGHT 6

// Print out an error message and exit.
static void fail(char const *message) {
  fprintf(stderr, "%s\n", message);
  exit(1);
}

// Print out a usage message, then exit.
static void usage() {
  printf("usage: square <workers>\n");
  printf("       square <workers> report\n");
  exit(1);
}

// Size of the grid of characters.
int rows, cols;

// Maximum width of a row.  Makes it easier to allocate the whole
// grid contiguously.
#define MAX_WIDTH 16384

// Type used for a row of the grid.  Makes it easier to declare the
// grid as a pointer.
typedef char Row[MAX_WIDTH];

// Grid of letters.
Row *grid;

// Read the grid of characters.
void readGrid() {
  // Read grid dimensions.
  scanf("%d%d", &rows, &cols);
  if (cols > MAX_WIDTH) {
    fprintf(stderr, "Input grid is too wide.\n");
    exit(EXIT_FAILURE);
  }

  // Make space to store the grid as a big, contiguous array.
  grid = (Row *)malloc(rows * sizeof(Row));

  // Read each row of the grid as a string, then copy everything
  // but the null terminator into the grid array.
  char buffer[MAX_WIDTH + 1];
  for (int r = 0; r < rows; r++) {
    scanf("%s", buffer);
    memcpy(grid[r], buffer, cols);
  }
}

/* keeps in track of the current row */
int volatile currentRow = 0;
/* number of workers */
int numWorker;
/* number of current running worker */
int volatile currentWorker;

/**
  * Checks individual 6 by 6 squares to see if it is a valid square.
  *
  * @param squares - individual squares to check
  * @return true if this square is valid
  */
bool checkSquare(char squares[6][6]) { //TODO algorithm for checking to see if every alphabet exists
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

/* Global variable that holds true if user passed in report argument */
bool report;

/**
  * Searches and finds 6 x 6 square in the grid.
  *
  * @return number of squares found
  */
int findSquares() {
  int totalSquares = 0;
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

        totalSquares++;
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
  return totalSquares;
}

/**
  * Entry point for this program.
  *
  * @param argc number of arguments
  * @param argv list of arguments
  * @return exit status
  */
int main(int argc, char *argv[]) {
  report = false;
  int workers = 4; //number of child processes to create

  // Parse command-line arguments.
  if (argc < 2 || argc > 3)
    usage();

  if (sscanf(argv[1], "%d", &workers) != 1 || workers < 1)
    usage();

  // If there's a second argument, it better be "report"
  if (argc == 3) {
    if (strcmp(argv[2], "report") != 0)
      usage();
    report = true;
  }

  readGrid();

  // ...
  numWorker = workers;
  int totalSquares = 0;
  int pfd[2]; //read and write pipe
  pid_t pids[workers];
  currentWorker = 0;
  for (int i = 0; i < workers; i++) {
    if (pipe(pfd) != 0) {
      fail("Can't create pipe");
    }

    if ((pids[i] = fork()) == -1) {
      fail("Cannot create child");
    } else if (pids[i] == 0) { //child
      close(pfd[0]); //close the reading end of the pipe
      int numOfSquares = findSquares();
      lockf(pfd[1], F_LOCK, 0);
      char sNum[6];
      sprintf(sNum, "%d", numOfSquares); //converts number of squares to string
      //send the number of squares to parent
      int num = write(pfd[1], sNum, 6);
      if (num < 0) {
        fail("Cannot write into pipe");
      }
      lockf(pfd[1], F_ULOCK, 0);
      close(pfd[1]);
      return EXIT_SUCCESS;
    } else { //parent
      close(pfd[1]); //close the writing end of the file
      //read from the pipe
      int numOfSquares;
      char sNum[6];
      if (read(pfd[0], sNum, 6) < 0) { //reads from the pipe
        fail("Error reading from the pipe");
      }
      numOfSquares = atoi(sNum); //converts string to int
      totalSquares += numOfSquares;
    }
    currentWorker++;
  }
  // wait for children to exit
  int status;
  while (workers > 0) {
    wait(&status);
    workers--;
  }
  char totalSquareString[6];
  sprintf(totalSquareString, "%d\n", totalSquares);
  write(STDOUT_FILENO, "Squares: ", 10);
  write(STDOUT_FILENO, totalSquareString, strlen(totalSquareString));
  return EXIT_SUCCESS;
}
