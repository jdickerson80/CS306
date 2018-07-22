#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <pwd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define true ( 1 )
#define false ( 0 )
#define BufferSize ( 4096 )
#define PORT ( 3333 )

#define HelloMessage ( "<remps>" )
#define HelloMessageLength ( strlen( HelloMessage ) )
#define SECRET ( "<CS30618spr>" )
#define SECRETSIZE ( strlen( SECRET ) )
#define ReadyMessage ( "<ready>" )
#define CPUMessage ( "<cpu>" )
#define MemMessage ( "<mem>" )

#define IPAddressArgv ( 1 )
#define DirectiveArgv ( 2 )
#define UsernameArgv ( 3 )

// macro function to close socket, print error, and exit
#define EXITANDPRINTERROR( error, socket ) \
({	close( socket ); fprintf( stderr, error ); exit( EXIT_FAILURE );	})
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
	invalid
} Directive;

/**
**************************
* Prototype Declarations *
**************************
*/

/**
 * @brief printUsage prints the program usage
 */
void printUsage();

/**
 * @brief getDirective gets the user's directive
 * @param arg command line argument
 * @return current directive
 */
Directive getDirective( char* arg );

/**
*************************
*			Main		*
*************************
*/
int main( int argc, char* argv[] )
{
	// check for proper arguments
	if ( argc < 2 )
	{
		// nope, so print usage and exit
		printUsage();
		exit( EXIT_FAILURE );
	}

	// get the user's directive
	Directive directive = getDirective( argv[ DirectiveArgv ] );

	// check if it is valid
	if ( directive == invalid )
	{
		printUsage();
		exit( EXIT_FAILURE );
	}

	// initialize variables
	char* serverIPAddress = argv[ IPAddressArgv ];
	char receiveBuffer[ BufferSize ];
	struct sockaddr_in serverAddress;
	struct passwd* userInfo;

	// setup the address
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons( PORT );
	inet_aton( serverIPAddress, &serverAddress.sin_addr );

	// create the socket
	int localSocket = socket( AF_INET, SOCK_STREAM, 0 );

	// set the socket options
	int i = 1;
	setsockopt( localSocket, SOL_SOCKET, SO_REUSEADDR, &i, sizeof( i ) );

	// connect to the server
	if ( connect( localSocket, (struct sockaddr*)&serverAddress, sizeof( serverAddress ) ) < 0 )
	{
		// had an error so close the socket and print the error to stderror
		EXITANDPRINTERROR( "Error connecting socket\n", localSocket );
	}

	// receive the hello message
	int messageSize = recv( localSocket, receiveBuffer, HelloMessageLength, 0 );

	// check if the hello message is correct
	if ( strcmp( receiveBuffer, HelloMessage ) )
	{
		// had an error so close the socket and print the error to stderror
		EXITANDPRINTERROR( "Incorrect hello message\n", localSocket );
	}

	// send the secret message
	messageSize = send( localSocket, SECRET, strlen( SECRET ), 0 );

	// check if the secret message was sent correctly
	if ( messageSize != SECRETSIZE )
	{
		// had an error so close the socket and print the error to stderror
		EXITANDPRINTERROR( "Could not send secret key\n", localSocket );
	}

	// receive the ready message
	messageSize = recv( localSocket, receiveBuffer, strlen( ReadyMessage ), 0 );

	// check if it is the correct secret message
	if ( strcmp( receiveBuffer, ReadyMessage ) )
	{
		// had an error so close the socket and print the error to stderror
		EXITANDPRINTERROR( "Received incorrect ready message\n", localSocket );
	}

	// handle the directive
	switch ( directive )
	{
	case cpu:
		messageSize = send( localSocket, CPUMessage, strlen( CPUMessage ), 0 );

		// check if the message was sent correctly
		if ( messageSize != strlen( CPUMessage ) )
		{
			// had an error so close the socket and print the error to stderror
			EXITANDPRINTERROR( "Could not send cpu directive\n", localSocket );
		}
		break;
	case mem:
		messageSize = send( localSocket, MemMessage, strlen( MemMessage ), 0 );

		// check if the message was sent correctly
		if ( messageSize != strlen( CPUMessage ) )
		{
			// had an error so close the socket and print the error to stderror
			EXITANDPRINTERROR( "Could not send cpu directive\n", localSocket );
		}
		break;
	case user:
		// get the user's id
		userInfo = getpwuid( getuid() );
		snprintf( receiveBuffer, BufferSize, "<user:%s>", userInfo->pw_name );
		messageSize = send( localSocket, receiveBuffer, strlen( receiveBuffer ), 0 );

		// check if the message was sent correctly
		if ( messageSize != strlen( receiveBuffer ) )
		{
			// had an error so close the socket and print the error to stderror
			EXITANDPRINTERROR( "User ID did not get sent\n", localSocket );
		}

		break;
	default: // just here to avoid the warning about not handling invalid
		break;
	}

	// receive the message
	while ( ( messageSize = recv( localSocket, receiveBuffer, BufferSize, 0 ) ) > 0 )
	{
		// print it to stdout
		write( STDOUT_FILENO, receiveBuffer, messageSize );
	}

	// close the socket
	close( localSocket );

	// return exit status
	return EXIT_SUCCESS;
}

void printUsage()
{
	fprintf( stderr,
			"remps SERVER_IP_ADDRESS [user | cpu | mem]\n"
			"where:\n"
			"\tSERVER_IP_ADDRESS – is the “IP address” of the server in dotted quad format\n"
			"\tuser – optional directive to show processes for user (the default)\n"
			"\tcpu – optional directive to show top processes by CPU usage\n"
			"\tmem – optional directive to show top process by memory usage\n" );
}

Directive getDirective( char* arg )
{
	if ( arg == NULL )
	{
		return cpu;
	}

	if ( !strcmp( arg, "cpu" ) )
	{
		return cpu;
	}
	else if ( !strcmp( arg, "mem" ) )
	{
		return mem;
	}
	else if ( !strcmp( arg, "user" ) )
	{
		return user;
	}

	return invalid;
}
