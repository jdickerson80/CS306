#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
**************************
* Enum Declarations		 *
**************************
*/

typedef enum StyleDef_t
{
	AllLines	= 'a',
	NoLines		= 'n',
	NonEmpty	= 't'
} StyleDef;

typedef enum UsageArguments_t
{
	Help		= 'h',
	LineWidth	= 'w',
	String		= 's',
	Style		= 'b',
} UsageArguments;

/**
**************************
* Prototype Declarations *
**************************
*/
/**
 * @brief fgetchar gets a character from the buffer
 * @param fd file to read from
 * @return character
 */
int fgetchar( int fd );

/**
* @brief fgetline is to read and return the next line from the open file stream fpntr.
* – It is to return the line as a valid C string.
* – It should return NULL if an I/O error occurs during its execution or if it immediately encounters the file-end.
* @param fd file descriptor
* @return
*/
char* fgetline( int fd );

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
int process_stream( int fd, StyleDef style, char* separator, int lineNumberWidth );


/**
**************************
*		Main			 *
**************************
*/
int main( int argc, char* argv[] )
{
	// init variables
	int error = 0;
	int fileArguement;
	int lineNumberWidth = 6;
	int	opt = 0;
	char* separator = "\t";
	int style = NonEmpty;

	// setup the long options0
	static struct option longOptions[] =
	{
		{ "body-numbering",		required_argument,	0, 'b' },
		{ "help",				no_argument,		0, 'h' },
		{ "number-separator",	required_argument,	0, 's' },
		{ "number-width",		required_argument,	0, 'w' },
		{ 0,					0,					0,	0  }
	};

	// parse the user's arguements
	while ( ( opt = getopt_long( argc, argv, "b:s:w:", longOptions, NULL ) ) != -1 )
	{
		switch ( opt )
		{
		case Help:
			fprintf( stdout, getUsage() );
			exit( EXIT_SUCCESS );
			break;

		case LineWidth:
			lineNumberWidth = atoi( optarg );
			break;
		case String:
			separator = optarg;
			break;

		case Style:
			style = *optarg;
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

int fgetchar( int fd )
{
#define CHARACTER_BUFFER_SIZE ( 512 )
	// initialize the local variables
	static char buffer[ CHARACTER_BUFFER_SIZE ];
	static int numbersRead = 0;
	static int position = 0;

	// if the position is >= numbers read
	if ( position >= numbersRead )
	{
		// fill in the buffer
		numbersRead = read( fd, (void*)buffer, CHARACTER_BUFFER_SIZE );

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

char* fgetline( int fd )
{
	// Macros
#define NEW_LINE_NULL_TERMINATOR_ADDER ( 2 )
#define INITIAL_BUFFER_SIZE ( 50 + NEW_LINE_NULL_TERMINATOR_ADDER )
#define BUFFER_SIZE_ADDER ( 10 )
	// init local variables
	char* buffer = (char*)malloc( INITIAL_BUFFER_SIZE );
	size_t bufferSize = INITIAL_BUFFER_SIZE;
	int position = 0;
	int nextChar;

	// get the next char and check for new line and EOF
	while ( ( nextChar = fgetchar( fd ) ) != '\n' && nextChar != EOF )
	{
		// if the buffer position PLUS \0 and \n is >= the buffer
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
		// since it will return null, free the buffer
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

const char* const getUsage()
{
	// Initialize all of the static variables
	static char usage[] = "The usage for the program is:\n"
						  "./mynl [-bSTYLE] [-sSTRING] [-wWIDTH] [FILE...]\n"
						  "Values for STYLE:\n\t– a meaning number all lines\n\t– n meaning number no lines\n\t– t meaning number only nonempty lines\n"
						  "Default STYLE if -b not provided:\tt\n"
						  "Default STRING if not provided is:\ttab ('\\t’)\n"
						  "Default WIDTH if not provided is:\t6\n";
	return usage;
}

int process_stream( int fd, StyleDef style, char* separator, int lineNumberWidth )
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
