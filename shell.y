
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT NEWLINE PIPE LESS CAT AND GREATAMP

%union	{
		char   *string_val;
	}

%{
	/* With the use of extern "C" 
	the C++ compiler ensures that 
	the code is unmangled as a C compiler does. */ 
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	simple_command
	|commands simple_command
	;


simple_command:	
	recursive_commands io_modifier_list background_identifier NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;

recursive_commands:
	command_and_args
	|recursive_commands command_and_args
	;


command_and_args:
	command_word arg_list{
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

command_word:

	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	|PIPE WORD {
		               printf("   Yacc: insert command \"%s\"\n", $2);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $2 );
	}
	;


arg_list:
	arg_list argument
	|
	 /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

io_modifier_list:
	io_modifier_list iomodifier
	|
	;

iomodifier:
	iomodifier_opt
	|iomodifier_input
	;

iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	|CAT WORD{
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._append= 1;
	}
	|GREATAMP WORD{
		printf("   Yacc: insert logFile \"%s\"\n", $2);
		Command::_currentCommand._errFile = $2;
	}
	;



iomodifier_input:
	LESS WORD {
		printf("   Yacc: insert Input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	;


background_identifier:
	AND {
		
		Command::_currentCommand._background = 1;
	}
	|
	;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
