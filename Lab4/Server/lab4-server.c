#define _GNU_SOURCE // needed to use pipe2
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

#define true ( 1 )
#define false ( 0 )
#define BufferSize ( 4096 )
#define PORT ( 3333 )

#define HelloMessage ( "<remps>" )
#define HelloMessageLength ( strlen( HelloMessage ) )
#define SECRET ( "<CS30618spr>" )
#define SECRETSIZE ( strlen( SECRET ) )
#define ReadyMessage ( "<ready>" )

#define ReadyMessage ( "<ready>" )
#define CPUMessage ( "<cpu>" )
#define MemMessage ( "<mem>" )

/**
**************************
*		Enum			 *
**************************
*/
typedef enum Directive_t
{
	cpu = 0,
	mem,
	user,
} Directive;

/**
**************************
*		Structs			 *
**************************
*/

/**
**************************
* Prototype Declarations *
**************************
*/
/**
 * @brief handle_client should be run by the server to handle each client/connection.
 * @param connect_fd is the file descriptor of the socket associated with a connection to a
 *	particular client.
 */
void handle_client( int connect_fd );

/**
 * @brief getDirective gets the user's directive
 * @param arg command line argument
 * @return current directive
 */
Directive getDirective( char* arg );

/**
 * @brief getUserName get the user's name from the client
 * @param buffer to hold the user's name
 * @param arg string to get the user's name from
 */
void getUserName( char* buffer, char* arg );
/**
*************************
*			Main		*
*************************
*/
int main( int argc, char* argv[] )
{
	// create the server address
	struct sockaddr_in serverAddress = { AF_INET, htons( PORT ), { htonl( INADDR_ANY ) }};
	socklen_t socketLength;

	// create the socket
	int serverSocket = socket( AF_INET, SOCK_STREAM, 0 );
	int i = 1;
	setsockopt( serverSocket, SOL_SOCKET, SO_REUSEADDR, &i, sizeof( i ) );

	// bind the address to the socket
	if ( bind( serverSocket, (struct sockaddr*) &serverAddress, sizeof( serverAddress ) ) == -1 )
	{
		// had an error, so exit the program
		fprintf( stderr, "Error: %s while trying to bind socket\n", strerror( errno ));
		close( serverSocket );
		exit( EXIT_FAILURE );
	}

	// ignore the sigpipe error
	signal( SIGPIPE, SIG_IGN );

	// set the socket into listen mode
	if ( listen( serverSocket, 10 ) == -1 )
	{
		// had an error, so exit the program
		fprintf( stderr, "Error: %s while listening\n", strerror( errno ));
		close( serverSocket );
		exit( EXIT_FAILURE );
	}

	// client address
	struct sockaddr_in clientAddress;

	while ( true )
	{
		// accept connections
		int acceptedSocket = accept( serverSocket, (struct sockaddr*)&clientAddress, &socketLength );

		// if there is a valid client
		if ( acceptedSocket > 0 )
		{
			// handle the client
			handle_client( acceptedSocket );

			// close the socket
			close( acceptedSocket );
		}
		else // bad client connection
		{
			// print the error
			fprintf( stderr, "%s\n", strerror( errno ) );
		}

		// do it all over again
	}

	// close the socket
	close( serverSocket );

	// return exit status
	return EXIT_SUCCESS;
}

void handle_client( int connect_fd )
{
	// init the buffer and variables
	char receiveBuffer[ BufferSize  ] = {0};
	Directive directive = cpu;

	// send the hello message
	int messageSize = send( connect_fd, HelloMessage, HelloMessageLength, 0 );

	// check for errors
	if ( messageSize != HelloMessageLength  )
	{
		// print error and return
		fprintf( stderr, "Error in the hello message\n");
		return;
	}

	// receive the secret
	messageSize = recv( connect_fd, receiveBuffer, BufferSize, 0 );

	// check if it is correct
	if ( strcmp( receiveBuffer, SECRET ) )
	{
		// print error and return
		fprintf( stderr, "Error in the secret message\n");
		return;
	}

	// send the ready message
	messageSize = send( connect_fd, ReadyMessage, strlen( ReadyMessage ), 0 );

	// check if it is correct
	if ( messageSize != strlen( ReadyMessage ) )
	{
		// print error and return
		fprintf( stderr, "Error in the ready message\n");
		return;
	}

	// clear the buffer
	memset( receiveBuffer, 0, messageSize );

	// receive the directive
	messageSize = recv( connect_fd, receiveBuffer, BufferSize, 0 );

	// create the proper pipes
	int pipeFileDescriptors[ 2 ] = { 0 };
	pipe2( pipeFileDescriptors, O_NONBLOCK | O_CLOEXEC );
	dup2( pipeFileDescriptors[ STDOUT_FILENO ], STDOUT_FILENO );
	dup2( pipeFileDescriptors[ STDOUT_FILENO ], STDERR_FILENO );

	// get the directive
	directive = getDirective( receiveBuffer );

	// init more variables
	int returnValue = 0;
	char userName[ 255 ] = {0};

	// handle the directive
	switch ( directive )
	{
	case user:
		// get the user's name
		getUserName( userName, receiveBuffer );

		// put it in the argument
		snprintf( receiveBuffer, BufferSize, "ps -u %s -NT -o pid,ppid,%%cpu,%%mem,args", userName );

		// run the shell
		returnValue = system( receiveBuffer );
		break;
	case cpu:
		// run the shell
		returnValue = system( "ps -NT -o pid,ppid,%cpu,%mem,args --sort -%cpu | head" );
		break;
	case mem:
		// run the shell
		returnValue = system( "ps -NT -o pid,ppid,%cpu,%mem,args --sort -%mem | head" );
		break;
	}

	// check if system ran correctly. extraneous!
	if ( returnValue < 0 )
	{
		fprintf( stderr, "Error: %s while running ps command\n", strerror( errno ) );
	}
	else // everything good
	{
		// send the data to the client
		int numberRead;
		while ( ( numberRead = read( pipeFileDescriptors[ 0 ], receiveBuffer, BufferSize ) ) > 0 )
		{
			send( connect_fd, receiveBuffer, numberRead, 0 );
		}
	}
}

Directive getDirective( char* arg )
{
	printf("%s", arg );
	if ( !strcmp( arg, "<cpu>" ) )
	{
		return cpu;
	}
	else if ( !strcmp( arg, "<mem>" ) )
	{
		return mem;
	}
	else if ( strstr( arg, ":" ) )
	{
		return user;
	}

	return cpu;
}

void getUserName( char* buffer, char* arg )
{
	char* s = strstr( arg, ":" );
	int i = strlen( arg ) - strlen( s ) + 1;
	int j = 0;

	while ( arg[ i ] != '>' )
	{
		buffer[ j++ ] = arg[ i++ ];
	}

	buffer[ ++j ] = '\0';
}
