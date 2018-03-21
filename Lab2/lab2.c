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
**************************
* Prototype declarations *
**************************
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
* @param lineNumberWidth the beginning line width
* @return EXIT_FAILURE for failure and EXIT_SUCCESS for success
*/
int process_stream( int fd, Style style, char* separator, int lineNumberWidth );

/**
* @brief fgetline is to read and return the next line from the open file stream fpntr.
* – It is to return the line as a valid C string.
* – It should return NULL if an I/O error occurs during its execution or if it immediately encounters the file-end.
* @param fd file descriptor
* @return
*/
char* fgetline( int fd );

/**
 * @brief fgetchar gets a character from the buffer
 * @param fd file to read from
 * @return character
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

	// set the file arg to the current index
	fileArguement = optind;

	// if the there is file paths given in the arguement
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
				// set the error, and restart the loop to open the next file
				fprintf( stderr, "Invalid file arguement given.\n" );
				error = 1;
				continue;
			}

			// process the stream
			if ( process_stream( fileDescriptor, style, separator, lineNumberWidth ) == EXIT_FAILURE )
			{
				error = 1;
			}

			// close the file
			close( fileDescriptor );
		}
	}
	else // no files in the argument
	{
		// process the stream from stdin
		process_stream( STDIN_FILENO, style, separator, lineNumberWidth );
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

int process_stream( int fd, Style style, char* separator, int lineNumberWidth )
{
	// local variables
	static unsigned int lineCounter = 0;
	char* line;

	// calculate the blank line width
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
	// Macros
#define NEW_LINE_NULL_TERMINATOR_ADDER ( 2 )
#define INITIAL_BUFFER_SIZE ( 10 + NEW_LINE_NULL_TERMINATOR_ADDER )
#define BUFFER_SIZE_ADDER ( INITIAL_BUFFER_SIZE / 4 )
	// init local variables
	char* buffer = (char*)malloc( INITIAL_BUFFER_SIZE );
	size_t bufferSize = INITIAL_BUFFER_SIZE;
	int position = 0;
	int nextChar;

	// get the next char and check for new line and EOF
	while ( ( nextChar = fgetchar( fd ) ) != '\n' && nextChar != EOF )
	{
		// if the position PLUS \0 and \n is empty
		if ( position + NEW_LINE_NULL_TERMINATOR_ADDER >= bufferSize )
		{
			// add the adder to the current buffer size
			bufferSize += BUFFER_SIZE_ADDER;

			// reallocate the new memory
			buffer = (char*)realloc( (void*)buffer, bufferSize );
		}

		// increment the position and set it equal to the next char
		buffer[ position++ ] = nextChar;
	}

	// if the next char is eof and the position is zero or an error
	if ( nextChar == EOF && ( position == 0 || errno ) )
	{
		// free the buffer since it will return null
		free( buffer );

		// return null
		return NULL;
	}

	// found the end of the line, so format to make a correct string
	buffer[ position++ ] = '\n';
	buffer[ position++ ] = '\0';

	// return buffer
	return buffer;
}

int fgetchar( int fd )
{
#define CHARACTER_BUFFER_SIZE ( 512 )
	// initialize the local variables
	static char buffer[ CHARACTER_BUFFER_SIZE ];
	static size_t bufferSize = sizeof( buffer) / sizeof( buffer[ 0 ] );
	static int position = 0;
	static int numbersRead = 0;

	// if the position is >= numbers read
	if ( position >= numbersRead )
	{
		// fill in the buffer
		numbersRead = read( fd, (void*)buffer, bufferSize );

		// error in numbers read
		if ( numbersRead <= 0 )
		{
			// return eof
			return EOF;
		}
		else // all good, so reset the position
		{
			position = 0;
		}
	}

	// return the buffer
	return buffer[ position++ ];
}
