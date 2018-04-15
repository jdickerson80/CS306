#define _GNU_SOURCE // needed to use pipe2
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#define true ( 1 )
#define false ( 0 )
#define FileString ( "_file_" )
#define BufferSize ( 4096 )

/**
**************************
*		Structs			 *
**************************
*/
typedef struct PipeInfo_t
{
	char* file;
	int inUse;
	pid_t pid;
	int pipeFileDescriptors[ 2 ];
} PipeInfo;

/**
**************************
* Prototype Declarations *
**************************
*/
/**
 * @brief printUsage prints the program's usage
 */
void printUsage();

/**
 * @brief printProcessHeader method prints out the desired header for each file
 * @param file that is being printed
 */
void printProcessHeader( char* file );

/**
 * @brief run_command_in_subprocess
 *  Does what is needed to be able to exec COMMAND on one FILE argument: calls
 *	fork() to create subprocess, and then in child calls execvp(), while in parent
 *	processs simply returns.
 * @param file file is the next of the FILE... arguments to be processed.
 * @param command the desired command to be run
 * @param commandArg arguments to be given to the command
 * @param pipeInfo pipe to use
 * @return true if there is an error, false if it is not
 */
int run_command_in_subprocess( char* file, char* command, char *commandArg, PipeInfo* pipeInfo );

/**
 * @brief process_terminated_subprocess retrieves and prints the output from a command subprocess that has terminated.
 * @param status is the status info from wait() that collected subprocess.
 * @param pid is the pid of the process to be terminated
 * @param numberOfCores cores to run
 * @param pipeInfo pipe to use
 * @return true if there is an error, false if it is not
 */
int process_terminated_subprocess( int status, int pid, int numberOfCores, PipeInfo* pipeInfo );

/**
*************************
*			Main		*
*************************
*/
int main( int argc, char* argv[] )
{
	// if there are not enough arguments
	if ( argc < 4 )
	{
		// print usage, and exit
		printUsage();
		exit( EXIT_FAILURE );
	}

	// variable for storing the file starting point
	size_t filePosition = 0;

	// loop through the arguments to find the starting position
	// of the files
	for ( size_t arg = 1; arg < argc; arg++ )
	{
		// is this string the _file_ string
		if ( !strcmp( argv[ arg ], FileString ) )
		{
			// it is, so capture it, and break the loop
			filePosition = arg;
			break;
		}
	}

	// check if the _file_ string is in the argument list
	if ( !filePosition )
	{
		// it is not, print the error and exit the program
		fprintf( stderr, "Must place the string _file_ before the first file argument\n" );
		exit( EXIT_FAILURE );
	}

	// we have a good run, so set up all of the local variables
	const size_t commandArgSize = filePosition - 2;
	const size_t numberOfFiles = argc - filePosition - 1;
	const int numberOfCores = atoi( argv[ 1 ] );
	size_t commandPosition = 0;
	int status;
	PipeInfo pipes[ numberOfCores ];
	char* command = argv[ 2 ];
	char commandArg[ commandArgSize ];
	int hasErrors = false;

	// build the command argument string
	for ( size_t i = 3; i < filePosition; ++i )
	{
		// get the argv string
		char* argument = argv[ i ];

		// init loop variable
		size_t looper = 0;

		// loop until the the null terminator is found
		while ( argument[ looper ] != '\0' )
		{
			// store the string
			commandArg[ commandPosition++ ] = argument[ looper++ ];
		}

		// add the null terminator
		commandArg[ commandPosition++ ] = '\0';
	}

	// init the desired number of pipes
	for ( size_t i = 0; i < numberOfCores; ++i )
	{
		// init the pipes
		pipes[ i ].pid = 0;
		pipes[ i ].inUse = false;
	}

	// counter for the number of running processes
	int runningProcessCounter = 0;

	// run the number of files
	for ( size_t i = 0; i < numberOfFiles; ++i )
	{
		// if the current count is < number of cores
		if ( runningProcessCounter < numberOfCores )
		{
			// find the first available pipe
			size_t availablePipe = 0;
			for ( ; availablePipe < numberOfCores; ++availablePipe )
			{
				// if the pipe is not in use
				if ( pipes[ availablePipe ].inUse == false )
				{
					// breaker
					break;
				}
			}

			// put the pipe into non blocking mode
			pipe2( pipes[ availablePipe ].pipeFileDescriptors, O_NONBLOCK );

			// run the command in a subprocess
			hasErrors |= run_command_in_subprocess( argv[ ++filePosition ], command, commandArg, &pipes[ availablePipe ] );

			// increment the process counter
			++runningProcessCounter;
		}
		else // if the current count is > number of cores
		{
			// wait for a subprocess to complete
			pid_t pid = wait( &status );

			// close down the subprocess
			hasErrors |= process_terminated_subprocess( status, pid, numberOfCores, pipes );

			// decrement the process counter
			--runningProcessCounter;
		}
	}

	// done forking processes, time to collect them
	for ( size_t i = 0; i < numberOfFiles; ++i )
	{
		// wait for a subprocess to complete
		pid_t pid = wait( &status );

		// close down the subprocess
		hasErrors |= process_terminated_subprocess( status, pid, numberOfCores, pipes );
	}

	// return exit status
	return hasErrors ? EXIT_FAILURE : EXIT_SUCCESS;
}

void printUsage()
{
	static const char* usage = "parcmds NUMCORES COMMAND... _files_ FILE...\n"
							   "NUMCORES – the maximum number of cores/CPUs to use simultaneously to run commands\n"
							   "COMMAND... – the command/program to be run, along with its options, etc. up to final file argument\n"
							   "_files_ – token to indicate start of file list\n"
							   "FILE... – a list of file names or pathnames.\n";
	printf( "%s", usage );
}

int run_command_in_subprocess( char* file, char* command, char* commandArg, PipeInfo *pipeInfo )
{
	pid_t pid;

	// fork and check for error
	if ( ( pid = fork() ) == -1 )
	{
		// print the error
		fprintf( stderr, "%s", "Fork failed, exiting\n " );

		// caller alert there is an error
		return true;
	}

	// is this the child
	if ( pid == 0 )
	{
		// it is, so set up the command string array
		char* newCommand[ 4 ];
		newCommand[ 0 ] = command;
		newCommand[ 1 ] = commandArg;
		newCommand[ 2 ] = file;
		newCommand[ 3 ] = NULL;

		// duplicate the std out file desciptor
		dup2( pipeInfo->pipeFileDescriptors[ STDOUT_FILENO ], STDOUT_FILENO );

		// close the file descriptors
		close( pipeInfo->pipeFileDescriptors[ STDIN_FILENO ] );
		close( pipeInfo->pipeFileDescriptors[ STDOUT_FILENO ] );

		// execute the command
		execvp( command, newCommand );

		// command returned, so alert the caller
		return true;
	}
	else // this is the parent
	{
		// capture the file name
		char fileName[ strlen( file ) + 1 ];
		strcpy( fileName, file );
		pipeInfo->file = file;

		// capture the pid and set the inUse flag to true
		pipeInfo->pid = pid;
		pipeInfo->inUse = true;
		return false;
	}
}

int process_terminated_subprocess( int status, int pid, int numberOfCores, PipeInfo* pipeInfo  )
{
	// check for error
	if ( WIFEXITED( status) && WEXITSTATUS( status ) != EXIT_SUCCESS )
	{
		fprintf( stderr, "A process did not complete successfully\n" );
		return true;
	}

	// if this pid has been collected
	if ( pid <= 0 )
	{
		// just return
		return false;
	}

	// init the variables
	size_t pipeUsed = 0;
	char buff[ BufferSize ];
	int nread;

	// find the pipe used for the given pid
	for ( ; pipeUsed < numberOfCores; ++pipeUsed )
	{
		// is this the pipe used for the pid
		if ( pipeInfo[ pipeUsed ].pid == pid )
		{
			break;
		}
	}

	// print the header
	printProcessHeader( pipeInfo[ pipeUsed ].file );

	// read in 4096 bytes
	while ( ( nread = read( pipeInfo[ pipeUsed ].pipeFileDescriptors[ STDIN_FILENO ], buff, BufferSize ) ) > 0 )
	{
		// write out the output
		write( STDOUT_FILENO, buff, nread );
	}

	// close down the pipe, and set the inUse flag to false
	close( pipeInfo[ pipeUsed ].pipeFileDescriptors[ STDIN_FILENO ] );
	close( pipeInfo[ pipeUsed ].pipeFileDescriptors[ STDOUT_FILENO ] );
	pipeInfo[ pipeUsed ].inUse = false;
	return false;
}

void printProcessHeader( char* file )
{
	printf( "--------------------\n" );
	printf( "Output from: %s\n", file );
	printf( "--------------------\n" );
}
