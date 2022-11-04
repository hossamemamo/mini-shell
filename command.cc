
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

#include <fcntl.h>	// for creat
#include <signal.h> // to catch CTRL+C
#include "command.h"
const char *ls = "ls";
// char * commands_list[]={"httpd","cat","grep","ls","cp","mv","rm","rmdir","mkdir","find","awk","tail","head","pwd"};

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
	if (_numberOfAvailableArguments == _numberOfArguments + 1)
	{
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **)realloc(_arguments,
									  _numberOfAvailableArguments * sizeof(char *));
	}

	_arguments[_numberOfArguments] = argument;

	// Add NULL argument at the end
	_arguments[_numberOfArguments + 1] = NULL;

	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
	if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
	{
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
													_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
	}

	_simpleCommands[_numberOfSimpleCommands] = simpleCommand;
	_numberOfSimpleCommands++;
}

void Command::clear()
{
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			free(_simpleCommands[i]->_arguments[j]);
		}

		free(_simpleCommands[i]->_arguments);
		free(_simpleCommands[i]);
	}

	if (_outFile)
	{
		free(_outFile);
	}

	if (_inputFile)
	{
		free(_inputFile);
	}

	if (_errFile)
	{
		free(_errFile);
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		printf("  %-3d ", i);
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
		}
	}

	printf("\n\n");
	printf("  Output       Input        Error        Background\n");
	printf("  ------------ ------------ ------------ ------------\n");
	printf("  %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
		   _inputFile ? _inputFile : "default", _errFile ? _errFile : "default",
		   _background ? "YES" : "NO");
	printf("\n\n");
}

void Command::execute()
{
	// Don't do anything if there are no simple commands
	if (_numberOfSimpleCommands == 0)
	{
		prompt();
		return;
	}
	// The dup() system call creates a copy of a file descriptor.
	int defaultin = dup(0);
	int defaultout = dup(1);
	int defaulterr = dup(3);

	//////////////////  ls  //////////////////////////

	// Input:    defaultin
	// Output:   file
	// Error:    defaulterr

	// Create file descriptor
	// Clear to prepare for next command
	print();

	int input_status;
	if (_inputFile)
	{
		input_status = open(_currentCommand._inputFile, O_RDONLY);
	}
	else
	{
		input_status = dup(defaultin);
	}

	int output_status;
	int err_status;
	int pid;
	// Print contents of Command data structure
	// Create new process for "ls"
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{

		if (strcmp(_simpleCommands[i]->_arguments[0], "cd") == 0)
		{
			chdir(_simpleCommands[i]->_arguments[1]);
		}
		else
		{
			dup2(input_status, 0); // replace stdin with the inputsream
			close(input_status);
			if (i == _numberOfSimpleCommands - 1)
			{
				if (_outFile)
				{
					if (_append)
					{
						output_status = open(_outFile, O_RDWR | O_APPEND);
					}
					else
					{
						output_status = open(_outFile, O_WRONLY);
					}
				}
				else
				{
					output_status = dup(defaultout);
				}

				if (_errFile)
				{
					err_status = open(_errFile, O_RDWR | O_APPEND);
				}
				else
				{
					err_status = dup(defaulterr);
				}
			}
			else
			{
				int fpipe[2];
				pipe(fpipe);
				output_status = fpipe[1];
				input_status = fpipe[0];
			}

			dup2(output_status, 1); // replace std output with output stream
			close(output_status);

			dup2(err_status, 2); // redirect error path to file or same if nothing changed check above
			close(err_status);
			pid = fork();
			if (pid == 0)
			{
				// Child
				//  You can use execvp() instead if the arguments are stored in an array
				execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
				// exec() is not suppose to return, something went wrong

				perror("ls: exec ls");
				_exit(1);
			}
		}
	}
	dup2(defaultin, 0); // return it as it was
	dup2(defaultout, 1);
	dup2(defaulterr, 2);

	close(input_status);
	close(output_status);
	close(err_status);

	close(defaultin);
	close(defaultout);
	close(defaulterr);
	if (!_background)
	{
		printf("excuting process not in background\n");
		waitpid(pid, 0, 0);
	}

	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec

	clear();

	// Print new prompt
	prompt();

	// Shell implementation
}
void signal_handler(int sig)
{
	printf(" Cannot be terminated Using CTRL+C \n");
}
void Command::prompt()
{
	signal(SIGINT, signal_handler);
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int yyparse(void);

int main()
{
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}
