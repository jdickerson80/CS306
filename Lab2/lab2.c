#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>

typedef enum Style_t
{
	AllLines	= 'a',
	NoLines		= 'n',
	NonEmpty	= 't'
} Style;

/**
* Prototype declarations
*/
/**
* @brief getUsage gets the default usage of the program
*/
const char* const getUsage();

/**
* @brief process_stream() is to take an open file stream fpntr, and carry out appropriate
* actions on it to produce correct output for the file (given the specified options).
* @param fd
* @param style
* @param separator
* @return
*/
int process_stream( int fd, Style style, char* separator, int lineNumberWidth );

/**
* @brief fgetline is to read and return the next line from the open file stream fpntr.
* – It is to return the line as a valid C string.
* – It should return NULL if an I/O error occurs during its execution or if it immediately encounters the file-end.
* @param fd
* @return
*/
char* fgetline( int fd );

/**
 * @brief fgetchar
 * @param fd
 * @return
 */
int fgetchar( int fd );

/**
 * @brief main function
 * @param argc
 * @param argv
 * @return program exit status
 */
int main( int argc, char* argv[] )
{
	// init variables
	char* separator = "\t";
	int style = NonEmpty;
	int opt = 0;
	int error = 0;
	int fileArguement;
	int lineNumberWidth = 6;

	// parse the user's arguements
	while ( ( opt = getopt( argc, argv, "b:s:w:" ) ) != -1 )
	{
		switch ( opt )
		{
		case 'b':
			style = *optarg;
			break;

		case 's':
			separator = optarg;
			break;

		case 'w':
			lineNumberWidth = atoi( optarg );
			break;

		default: /* '?' */
			fprintf( stderr, getUsage() );
			exit( EXIT_FAILURE );
		}
	}

	fileArguement = optind;

	if ( fileArguement < argc )
	{
		// loop through the files, doing the nl thang
		for ( ; fileArguement < argc; ++fileArguement )
		{
			// open the file
			int fileDescriptor = open( argv[ fileArguement ], O_RDONLY );

			// incorrect file give, so set the error and continue
			if ( fileDescriptor == -1 )
			{
				fprintf( stderr, "Invalid file arguement given.\n" );
				error = 1;
				continue;
			}

			// process the stream
			if ( process_stream( fileDescriptor, style, separator, lineNumberWidth ) == EXIT_FAILURE )
			{
				// stream ran into a problem, so close the file, and set the error flag
				close( fileDescriptor );
				error = 1;
				break;
			}

			// close the file
			close( fileDescriptor );
		}
	}
	else
	{
		int fileDescriptor = STDIN_FILENO;
		process_stream( fileDescriptor, style, separator, lineNumberWidth );
		close( fileDescriptor );
	}

	// return the error code. <3 ternary
	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}

const char* const getUsage()
{
	// Initialize all of the static variables
	static char usage[] = "The usage for the program is:\n"
						  "./mynl [-bSTYLE] [-sSTRING] [-wWIDTH] [FILE...]\n"
						  "Values for STYLE:\n\t– a meaning number all lines\n\t– n meaning number no lines\n\t– t meaning number only nonempty lines\n"
						  "Default STYLE if -b not provided: t\n"
						  "Default STRING if not provided is: tab ('\\t’)\n";
	return usage;
}

int process_stream(int fd, Style style, char* separator, int lineNumberWidth )
{
	static unsigned int lineCounter = 0;
	char* line;

	int blankLineWidth = lineNumberWidth + strlen( separator );
	// switch on the specified style
	switch ( style )
	{
	case AllLines:
		while ( ( line = fgetline( fd ) ) != NULL )
		{
			printf( "%*d%s%s", lineNumberWidth, ++lineCounter, separator, line );
			free( line );
		}
		break;

	case NoLines:
		while ( ( line = fgetline( fd ) ) != NULL )
		{
			printf( "%*s%s", blankLineWidth, " ", line );
			free( line );
		}
		break;

	case NonEmpty:
		while ( ( line = fgetline( fd ) ) != NULL )
		{
			// if the line is empty
			if ( strlen( line ) == 1 )
			{
				// print the appropriate line
				printf("%*s\n", blankLineWidth, "" );
			}
			else // line is not empty, so print line number
			{
				printf( "%*d%s%s", lineNumberWidth, ++lineCounter, separator, line );
			}
			free( line );
		}
		break;
		// default case to catch user's garbage input...
	default:
		fprintf( stderr, "Incorrect style arguement given.\n%s", getUsage() );
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

char* fgetline( int fd )
{
#define NEW_LINE_NULL_TERMINATOR_ADDER ( 2 )
#define INITIAL_BUFFER_SIZE ( 1024 + NEW_LINE_NULL_TERMINATOR_ADDER )
#define BUFFER_SIZE_ADDER ( INITIAL_BUFFER_SIZE / 4 )
	char* buffer = (char*)malloc( INITIAL_BUFFER_SIZE );
	size_t bufferSize = INITIAL_BUFFER_SIZE;
	int position = 0;
	int nextChar;

	while ( ( nextChar = fgetchar( fd ) ) != '\n' && nextChar != EOF )
	{
		if ( position >= bufferSize )
		{
			bufferSize += BUFFER_SIZE_ADDER;
			buffer = (char*)realloc( (void*)buffer, bufferSize );
			printf("buff size %u ptr %p\n", bufferSize, buffer );
		}
		buffer[ position++ ] = nextChar;
	}


	if ( nextChar == EOF && ( position == 0 || errno ) )
	{
		return NULL;
	}

	// found the end of the line, so format to make a correct string
	buffer[ position++ ] = '\n';
	buffer[ position++ ] = '\0';

	// return line buffer
	return buffer;
}

int fgetchar( int fd )
{
#define CHARACTER_BUFFER_SIZE ( 512 )
	static char buffer[ CHARACTER_BUFFER_SIZE ];
	static size_t bufferSize = sizeof( buffer) / sizeof( buffer[ 0 ] );
	static int position = 0;
	static int numbersRead = 0;

	if ( position >= numbersRead )
	{
		numbersRead = read( fd, (void*)buffer, bufferSize );
		if ( numbersRead <= 0 )
		{
			return EOF;
		}
		else
		{
			position = 0;
		}
	}

	return buffer[ position++ ];
}
