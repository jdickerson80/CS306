#include "mynl.h"
#include <stdlib.h>
#include <string.h>

int main( int argc, char* argv[] )
{
	if ( argc == 1 )
	{
		fprintf( stderr, getUsage() );
		exit( EXIT_FAILURE );
	}

	char* separator = defaultSeparator();
	int style = NonEmpty;
	int opt = 0;

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

	if ( optind >= argc || argc - optind <= 1 )
	{
		fprintf( stderr, "Expected argument after option %d.\n%s", optind - 1, getUsage() );
		exit( EXIT_FAILURE );
	}

	for ( ; optind < argc; ++optind )
	{
		// loop through the files, printing them all
		FILE* file = fopen( argv[ optind ], "r" );

		if ( file == NULL )
		{
			fprintf( stderr, "Invalid file arguement given.\n" );
			exit( EXIT_FAILURE );
		}

		if ( process_stream( file, style, separator ) == EXIT_FAILURE )
		{
			exit( EXIT_FAILURE );
		}
	}
}
