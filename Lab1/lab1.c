#include "mynl.h"
#include <stdlib.h>
#include <string.h>

int main( int argc, char* argv[] )
{
	// init variables
	char* separator = defaultSeparator();
	int style = NonEmpty;
	int opt = 0;
	int error = 0;

	// catch if program is parsing input stream
	if ( argc == 1 )
	{
		process_stream( stdin, style, separator );

		// exit because the rest of the program cannot run if no arguements are given
		exit( EXIT_SUCCESS );
	}

	// parse the user's arguements
	while ( ( opt = getopt( argc, argv, "b:s:" ) ) != -1 )
	{
		switch ( opt )
		{
		case 'b':
			style = *optarg;
			break;
		case 's':
			separator = optarg;
			break;
		default: /* '?' */
			fprintf( stderr, getUsage() );
			exit( EXIT_FAILURE );
		}
	}

	// check if the user did not give an arguement after the a modifier
	if ( optind >= argc )
	{
		fprintf( stderr, "Expected argument after option %d.\n%s", optind - 1, getUsage() );
		exit( EXIT_FAILURE );
	}

	// loop through the files, doing the nl thang
	for ( ; optind < argc; ++optind )
	{
		// open the file
		FILE* file = fopen( argv[ optind ], "r" );

		// incorrect file give, so set the error and continue
		if ( file == NULL )
		{
			fprintf( stderr, "Invalid file arguement given.\n" );
			error = 1;
			continue;
		}

		// process the stream
		if ( process_stream( file, style, separator ) == EXIT_FAILURE )
		{
			// stream ran into a problem, so close the file, and set the error flag
			fclose( file );
			error = 1;
			break;
		}

		// close the file
		fclose( file );
	}

	// return the error code. <3 ternary
	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
