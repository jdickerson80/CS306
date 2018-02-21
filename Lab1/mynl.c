#include "mynl.h"
#include <stdlib.h>
#include <string.h>

static char usage[] = "The usage for the program is:\n"
					  "./mynl [-bSTYLE] [-sSTRING] [FILE...]\n"
					  "Values for STYLE:\n\t– a meaning number all lines\n\t– n meaning number no lines\n\t– t meaning number only nonempty lines\n"
					  "Default STYLE if -b not provided: t\n"
					  "Default STRING if not provided is: tab ('\\t’)\n";

static char DefaultSeparator[] = "\t";

//Preprocessor constants:
#define MAX_LINE_LENGTH 1000

char* getUsage()
{
	return usage;
}

char* defaultSeparator()
{
	return DefaultSeparator;
}

int process_stream( FILE* fpntr, Style style, char* separator )
{
	char* line;
	unsigned int lineNumber = 1;

	switch ( style )
	{
	case AllLines:
		while ( ( line = fgetline( fpntr ) ) != NULL )
		{
			printf("%0*d%s%s", strlen( separator ), lineNumber, separator, line );
			lineNumber++;
		}
		break;

	case NoLines:
		while ( ( line = fgetline( fpntr ) ) != NULL )
		{
			printf("%*s", strlen( separator ), line );
		}
		break;

	case NonEmpty:
		while ( ( line = fgetline( fpntr ) ) != NULL )
		{
			if ( strlen( line ) == 1 )
			{
				printf("%-*s", strlen( separator ), line );
			}
			else
			{
				printf("%-*d%s%s", strlen( separator ), lineNumber, separator, line );
				lineNumber++;
			}
		}
		break;

	default:
		fprintf( stderr, "Incorrect style arguement given.\n%s", getUsage() );
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

char* fgetline( FILE* fpntr )
{
	static char line_buff[ MAX_LINE_LENGTH ];
	// use fgetc to get each character insead of using fgets
	size_t temp = 0;
	int next = 0;
	while ( ( next = fgetc( fpntr ) ) != '\n' && next != EOF )
	{
		line_buff[ temp++ ] = next;
	}

	if ( next == EOF )
	{
		return NULL;
	}

	line_buff[ temp++ ] = '\n';
	line_buff[ temp++ ] = '\0';
	return line_buff;
}
