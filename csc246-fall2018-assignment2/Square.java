import java.util.ArrayList;
import java.util.Scanner;

/**
 * CSC246 Assignment 2 Problem 3.
 *
 * @author John Choi
 * @since 10052018
 */
public class Square {

	private static volatile int numSquares = 0;
	private static volatile boolean report;
	private static char[][] board;
	private static int rows;
	private static int cols;
	private static Scanner input = new Scanner(System.in);
	private static volatile int currentRow = 0;
	private static volatile int currentWorker;

	/**
	 * Inner class that handles thread operation.
	 *
	 * @author John Choi
	 * @since 10052018
	 */
	static class SingleThread implements Runnable {

		private int numWorker;


		public SingleThread(int numWorker) {
			this.numWorker = numWorker;
		}

		/**
		 * Defines the thread's job.
		 */
		@Override
		public void run() {
			findSquares();
		}

		/**
		 * Returns true if the passed in 2d array is a valid square.
		 *
		 * @param square to validate
		 * @return true if square is valid
		 */
		private boolean isSquare(char[][] square) {
			ArrayList<Character> alphabets = new ArrayList<>();
			for (char i = 'a'; i <= 'z'; i++) {
				alphabets.add(i);
			}

			for (int i = 0; i < square.length; i++) {
				for (int j = 0; j < square[0].length; j++) {
					if (alphabets.contains(square[i][j])) {
						int idx = -1;
						for (int k = 0; k < alphabets.size(); k++) {
							if (alphabets.get(k) == square[i][j]) {
								idx = k;
								break;
							}
						}
						alphabets.remove(idx);
					}
				}
			}
			if (alphabets.size() == 0) {
				return true;
			}
			return false;
		}

		/**
		 * Searches and finds 6 x 6 square in the board.
		 */
		public void findSquares() {
			int totalSquaresFound = 0;
			currentRow = 0;
			char[][] square = new char[6][6];
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
							square[currentLocationA][currentLocationB++] = board[i][j];
						}

						currentLocationA++;
						currentLocationB = 0;
					}
					currentLocationA = 0;

					if (isSquare(square)) {
						if (report) {
							System.out.printf("%d %d\n", lowerBoundRow, lowerBoundCol);
						}

						totalSquaresFound++;
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
			numSquares += totalSquaresFound;
		}
	}

	/**
	 * Initializes the board.
	 */
	public static void initializeBoard() {
		rows = input.nextInt();
		cols = input.nextInt();
		input.nextLine();
		board = new char[rows][cols];
		for (int i = 0; i < rows; i++) {
			String thisline = input.nextLine(); //save one line
			for (int j = 0; j < cols; j++) {
				board[i][j] = thisline.charAt(j);
			}
		}
		input.close();
	}

	/**
	 * Main method for this program.
	 *
	 * @param args command-line arguments
	 */
	public static void main(String[] args) {
		report = false;
		if (args.length == 2) {
			report = true;
		} else if (args.length == 1) {
			report = false;
		} else {
			System.out.println("usage: java Square <number-of-threads> < filename.txt");
			System.out.println("       java Square <number-of-threads> report < filename.txt");
			System.exit(1);
		}

		initializeBoard();
		int numOfThreads = Integer.parseInt(args[0]);
		Thread[] threads = new Thread[numOfThreads];
		currentWorker = 0;
		for (int i = 0; i < numOfThreads; i++) {
			threads[i] = new Thread(new SingleThread(numOfThreads));
			threads[i].run();
			currentWorker++;
		}

		for (int i = 0; i < numOfThreads; i++) {
			try {
				threads[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		System.out.println("Squares: " + numSquares);
	}
}
