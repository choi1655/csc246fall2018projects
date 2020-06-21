import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Scanner;
import java.util.Random;
import java.util.ArrayList;
import java.util.Arrays;
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;
import java.security.PublicKey;
import java.security.KeyFactory;
import java.security.spec.X509EncodedKeySpec;
import java.security.GeneralSecurityException;
import java.util.Base64;

/** A server that keeps up with a public key for every user, along
    with a board for placing letters, like scrabble. */
public class Server {
  /** Port number used by the server */
  public static final int PORT_NUMBER = 26262;

  /** Original state of the board, for resetting at the start of a game. */
  private char[][] template;

  /** Current board, a 2D array of characters. */
  private char[][] board;

  /** Record for an individual user. */
  private static class UserRec {
    // Name of this user.
    String name;

    // This user's public key.
    PublicKey publicKey;

    // Current score for this users.
    int score;
  }

  /** List of all the user records. */
  private ArrayList< UserRec > userList = new ArrayList< UserRec >();

  /** Set the game board back to its initial state. */
  private void reset() {
    for ( int i = 0; i < board.length; i++ )
      for ( int j = 0; j < board[ i ].length; j++ )
        board[ i ][ j ] = template[ i ][ j ];

    for ( int i = 0; i < userList.size(); i++ )
      userList.get( i ).score = 0;
  }

  /** Read the initial board and all the users, done at program start-up. */
  private void readConfig() throws Exception {
    // First, read in the map.
    Scanner input = new Scanner( new File( "board.txt" ) );

    // Read in the initial state of the board.
    int height = input.nextInt();
    int width = input.nextInt();
    input.nextLine(); // Eat the rest of the first line.

    // Make the board state.
    template = new char [ height ][];
    for ( int i = 0; i < height; i++ )
      template[ i ] = input.nextLine().toCharArray();
    board = new char [ height ][ width ];

    // Read in all the users.
    input = new Scanner( new File( "passwd.txt" ) );
    while ( input.hasNext() ) {
      // Create a record for the next user.
      UserRec rec = new UserRec();
      rec.name = input.next();

      // Get the key as a string of hex digits and turn it into a byte array.
      String base64Key = input.nextLine().trim();
      byte[] rawKey = Base64.getDecoder().decode( base64Key );

      // Make a key specification based on this key.
      X509EncodedKeySpec pubKeySpec = new X509EncodedKeySpec( rawKey );

      // Make an RSA key based on this specification
      KeyFactory keyFactory = KeyFactory.getInstance( "RSA" );
      rec.publicKey = keyFactory.generatePublic( pubKeySpec );

      // Add this user to the list of all users.
      userList.add( rec );
    }

    // Reset the state ofthe game.
    reset();
  }

  /** Utility function to read a length then a byte array from the
      given stream.  TCP doesn't respect message boundaraies, but this
      is essientially a technique for marking the start and end of
      each message in the byte stream.  This can also be used by the
      client. */
  public static byte[] getMessage( DataInputStream input ) throws IOException {
    int len = input.readInt();
    byte[] msg = new byte [ len ];
    input.readFully( msg );
    return msg;
  }

  /** Function analogous to the previous one, for sending messages. */
  public static void putMessage( DataOutputStream output, byte[] msg ) throws IOException {
    // Write the length of the given message, followed by its contents.
    output.writeInt( msg.length );
    output.write( msg, 0, msg.length );
    output.flush();
  }

  /** Function to handle interaction with a client.  For a multi-threaded
      server, this should be done in a separate thread. */
  public void handleClient( Socket sock ) {
    try {
      // Get formatted input/output streams for this thread.  These can read and write
      // strings, arrays of bytes, ints, lots of things.
      DataOutputStream output = new DataOutputStream( sock.getOutputStream() );
      DataInputStream input = new DataInputStream( sock.getInputStream() );

      // Get the username.
      String username = input.readUTF();

      // Make a random sequence of bytes to use as a challenge string.
      Random rand = new Random();
      byte[] challenge = new byte [ 16 ];
      rand.nextBytes( challenge );

      // Make a session key for communiating over AES.  We use it later, if the
      // client successfully authenticates.
      byte[] sessionKey = new byte [ 16 ];
      rand.nextBytes( sessionKey );

      // Find this user.  We don't need to synchronize here, since the set of users never
      // changes.
      UserRec rec = null;
      for ( int i = 0; rec == null && i < userList.size(); i++ )
        if ( userList.get( i ).name.equals( username ) )
          rec = userList.get( i );

      // Did we find a record for this user?
      if ( rec != null ) {
        // Make sure the client encrypted the challenge properly.
        Cipher RSADecrypter = Cipher.getInstance( "RSA" );
        RSADecrypter.init( Cipher.DECRYPT_MODE, rec.publicKey );

        Cipher RSAEncrypter = Cipher.getInstance( "RSA" );
        RSAEncrypter.init( Cipher.ENCRYPT_MODE, rec.publicKey );

        // Send the client the challenge.
        putMessage( output, challenge );

        // Get back the client's encrypted challenge.
        // ...
        byte[] encryptedChallenge = new byte[16];
        encryptedChallenge = getMessage(input); //does not
        byte[] decryptedChallenge = RSADecrypter.doFinal(encryptedChallenge);

        if (!Arrays.equals(challenge, decryptedChallenge)) {
        	//error because it wasnt properly encrypted the challenge
          System.exit(1);
        }
        // Send the client the session key (encrypted with the client's public
        // key).
        // ...
        byte[] encryptedSessionKey = new byte[16];
        encryptedSessionKey = RSAEncrypter.doFinal(sessionKey);
        putMessage(output, encryptedSessionKey);

        // Make AES cipher objects to encrypt and decrypt with
        // the session key.
        // ...

        SecretKey AES_SessionKey = new SecretKeySpec(sessionKey, "AES");
        Cipher AESEncrypter = Cipher.getInstance("AES/ECB/PKCS5Padding");
        AESEncrypter.init(Cipher.ENCRYPT_MODE, AES_SessionKey);
        Cipher AESDecrypter = Cipher.getInstance("AES/ECB/PKCS5Padding");
        AESDecrypter.init(Cipher.DECRYPT_MODE, AES_SessionKey);

        // Get the first client command
        byte[] encryptedRequestraw = getMessage(input);
        byte[] encryptedRequest = Base64.getDecoder().decode(encryptedRequestraw.toString());
        String request = AESDecrypter.doFinal(encryptedRequest).toString();

        System.out.println(request);
        // Until the client asks us to exit.
        while ( ! request.equals( "exit" ) ) {
          StringBuilder reply = new StringBuilder();
          if (request.equals("board")) {
            System.out.println("Hit");
        	  reply.append("Board called");
          } else if (request.equals("place")) {
        	  reply.append("place called");
          } else {
        	  reply.append("Invalid command\n");
          }

          // For now, just send back a copy fo the request as a reply.
//          reply.append( request + "\n" );

          // Send the reply back to our client.
          byte[] encryptedReply = new byte[16];
          System.out.println("hello2");
          encryptedReply = AESEncrypter.doFinal(reply.toString().getBytes());
          System.out.println("hello3");
          putMessage( output, encryptedReply );

          // Get the next command.
          encryptedRequest = getMessage(input);
          request = AESDecrypter.doFinal(encryptedRequest).toString();
        }
      }
    } catch ( IOException e ) {
      System.out.println( "IO Error: " + e );
    } catch( GeneralSecurityException e ){
      System.err.println( "Encryption error: " + e );
    } finally {
      try {
        // Close the socket on the way out.
        sock.close();
      } catch ( Exception e ) {
      }
    }
  }

  /** Esentially, the main method for our server, as an instance method
      so we can access non-static fields. */
  private void run( String[] args ) {
    ServerSocket serverSocket = null;

    // One-time setup.
    try {
      // Read the map and the public keys for all the users.
      readConfig();

      // Open a socket for listening.
      serverSocket = new ServerSocket( PORT_NUMBER );
    } catch( Exception e ){
      System.err.println( "Can't initialize server: " + e );
      e.printStackTrace();
      System.exit( 1 );
    }

    // Keep trying to accept new connections and serve them.
    while( true ){
      try {
        // Try to get a new client connection.
        Socket sock = serverSocket.accept();

        // Handle interaction with the client
        handleClient( sock );
      } catch( IOException e ){
        System.err.println( "Failure accepting client " + e );
      }
    }
  }

  public static void main( String[] args ) {
    // Make a server object, so we can have non-static fields.
    Server server = new Server();
    server.run( args );
  }
}
