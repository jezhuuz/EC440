//E440 Operating Systems 
//Project 1
//Jenna Zhu

//to exit the shell, the user must type Ctrl-D (pressing the D button while holding control)

//fork()
//execvp()
//waitip() or wait()

#include <stdio.h>
#include <string.h>
#include <stdlib.h> //malloc and free
#include <unistd.h> //fork(), exec()
#include <fcntl.h> //file descriptor stuff
#define MAX_SIZE 512
#define TRUE 1
#define FALSE 0
#define WRITE_END 1
#define READ_END 0

//this array holds the "n" command along with "\0"
char check[2]; 

//character array of max size 512 for command line
char enterarray[MAX_SIZE];
//array that holds the labels of the type of token during parsing 
char *labels[MAX_SIZE];
//array that holds all the tokens after parsing
char *mytokens[MAX_SIZE];
//count of amount of tokens in an input command line
int tokencount;
//amount of pipes in the given command
int numpipes;

//this points to the string that is received from user input
char *input;
//char *newinput;
//if there is an input redirect in the command
int inputflag = 0;
//if there is an output redirect in the command
int outputflag = 0;
//if there is a & for a background process command
int backgroundflag = 0;
//flag for if there are any errors in the commandline i.e. multiple >, <, or &, etc
int cmdError = 0;
//pointer to input redirect file
char *infile;
//pointer to output redirect file
char *outfile;
//array that holds coordinates of the beginning and end of the tokens before and after pipes 
//(this is used later on for multiple piping commands)
int location[MAX_SIZE][2];
int total;

char *command;
//if there is an error in the command
int iserror;

//the print statement for the shell prompt
char *myprint = "my_shell> ";

//malloc for prompt inputs
void increasesize()
{
	input = malloc((sizeof(char)*MAX_SIZE));
	//newinput = malloc((sizeof(char)*MAX_SIZE));
}

//to print the shell prompt command header
void shellprompt()
{
	//printf("my_shell> ");
	printf("%s", myprint);
}

char my_parser()
{
	//this is to figure out the size of the array that holds every char in the input string
	int size = strlen(input);
	//the +1 is for the added \0 
	char NEWcommandline[MAX_SIZE + 1];

	int charcount = 0;
	int copycount = 0;

	//add spaces before and after each metacharacter in case there are no spaces between tokens/meta characters
	//copy array over to new, formatted array
	while(input[charcount] != '\0')
	{	//if current character is a metacharacter
		if((input[charcount] == '<')||(input[charcount] == '|')||(input[charcount] == '>')||(input[charcount] == '&'))
		{	
			//format the array so that there are spaces before and after each metacharacter 
			NEWcommandline[copycount] = ' ';
			NEWcommandline[copycount + 1] = input[charcount];
			NEWcommandline[copycount + 2] = ' ';
			copycount = copycount + 3;
		}

		else
		{

			NEWcommandline[copycount] = input[charcount];
			copycount++;
		}
		charcount++;
	}


	//char ** listpt = malloc(memsize * sizeof(char*));
	char *first = strtok(NEWcommandline, " \n");


	NEWcommandline[charcount+1] = '\0';

	tokencount = 0;
	int metachar = 0;

	//flag of what the previous token was: first, command, pipe, argument, output, input, background
	int previous = 0;

	//0 = first command
	//1 = command
	//2 = argument
	//3 = pipe |
	//4 = input redirect >
	//5 = output redirect <
	//6 = background &

//loop this til the char array hits \0
//"read" tokens and place the type/label of token into label array
	while(first != NULL)
	{

		int tokposition = 0;

		if(*first == '|')
		{
			tokposition = 1;
			metachar = 1;
			labels[tokencount] = "pipe";
		}

		else if(*first == '>')
		{
			tokposition = 2;
			metachar = 2;
			labels[tokencount] = "output redirect";
		}

		else if(*first == '<')
		{
			tokposition = 2;
			metachar = 2;
			labels[tokencount] = "input redirect";
		}	

		else if(*first == '&')
		{	
			metachar = 1;
			labels[tokencount] = "background";
			first = NULL;
			//return;
		}

		else if((tokposition == 2)&((metachar == 0)||(metachar == 2)))
		{
				labels[tokencount] = "argument";
		}

		else if((tokposition == 0) || (tokposition == 1))
		{
			metachar = 0;
			labels[tokencount] = "command";
		}
		else
		{
			labels[tokencount] = "invalid";
			tokposition = 0;
		}

		mytokens[tokencount] = first;
		printf(mytokens[tokencount]);
		printf("\n");
		tokencount++;
		first = strtok(NULL, " ");
	} 

	int toksec = 0;
	inputflag = 0;
	outputflag = 0;
	backgroundflag = 0;
	infile = "";
	outfile = "";
	numpipes = 0;
	location[0][0] = 0;

	int count;

	//loop through tokens and set flags for metacharacters and its locations
	for(count = 0; count < tokencount; count++)
	{
		if(strcmp(labels[count], "background") == 0)
		{
			location[toksec][1] = count - 1;
			backgroundflag = TRUE;
		}

		//if token is pipe, end current token section and start new
		else if(strcmp(labels[count], "pipe") == 0)
		{
			numpipes++;
			location[toksec][1] = count - 1;
			location[++toksec][0] = count + 1;
		}
		//if token is input redirect
		else if(strcmp(labels[count], "input redirect") == 0)
		{
			inputflag = TRUE;
			infile = mytokens[count + 1];
			location[toksec][1] = count - 1;
		}
		//if token is output redirect
		else if(strcmp(labels[count], "output redirect") == 0)
		{
			outputflag = TRUE;
			outfile = mytokens[count +  1];
			location[toksec][1] = count - 1;
		}
		//if token given is invalid/unrecognized
		else if(strcmp(labels[count], "invalid") == 0)
		{
			printf("ERROR: Invalid command. \n");
			iserror = TRUE;
		}
	}

	//finish the token coordinates
	if((backgroundflag == FALSE) && (inputflag == FALSE) && (outputflag == FALSE)) 
	{
		location[toksec][1] = tokencount - 1;
	}

}

int execute()
{
	//argument array
	char *arg[MAX_SIZE];
	//execution error
	int executionerr = 0;
	//wait error
	int waiterror = 0;
	//count of the pipe
	int pipecount = 0;
	//build the array to be passed into execvp() 
	int argcount = 0;
	//process ID for shell parent
	pid_t pid;
	//process ID for parent/children
	pid_t pidcont;
	//tokens left of the pipe
	int endpipe = 0; 
	//end of the pipe to the left (right before pipe)
	int frontpipe[2];

	int inputdesc = dup(STDIN_FILENO);
	int outputdesc = dup(STDOUT_FILENO);

	//if there is an input file
	if(inputflag)
	{
		inputdesc = open(infile, O_RDONLY);
		//cannot open file
		if(inputdesc == -1)
		{
			perror("ERROR");
		}

		endpipe = inputdesc;
	}
	//if there is an output file
	if(outputflag)
	{
		//open or create nonexisting file to write to
		outputdesc = creat(outfile, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		//cannot open or create file
		if(outputdesc == -1)
		{
			perror("ERROR");
		}
	}

	///set endpipe to defined input description
	endpipe = inputdesc;

	//in the case of no tokens
	if(tokencount == 0)
	{
		return 0;
	}

	//if there are no pipes in the input
	else if(numpipes == 0)
	{
		//place a NULL at end of mytokens
		mytokens[tokencount + 1] = (char*) NULL;
		int i;
		//clear the argument array of any previous arguments
		memset(arg, 0, sizeof(arg));
		//build the arg array
		for(i = location[pipecount][0]; i <= location[pipecount][1]; i++)
		{
			arg[argcount] = mytokens[i];
			argcount++;
		}
		
		//now fork the shell to create a child
		pid = fork();

		//if there is a forking error
		if(pid == -1)
		{
			perror("ERROR");
		}
		//execute command in the child process
		else if(pid == 0)
		{
			//dup input/output file descriptors
			dup2(inputdesc, STDIN_FILENO);
			dup2(outputdesc, STDOUT_FILENO);
			//execute
			execvp(arg[0], arg);
			perror("ERROR");
			exit(EXIT_FAILURE);
		}
		else if(pid != 0)
		{
			if(backgroundflag == FALSE)
			{
				//have process wait if no backgrounding
				waitpid(pid, &waiterror, 0);
			}
		}
	}
		//if multiple pipes
		else
		{	
			//fork
			pidcont = fork();

			if(pidcont == -1)
			{
				perror("ERROR");
				exit(EXIT_FAILURE);
			}
			//execute command when in child
			else if(pidcont == 0)
			{
				//while there are more pipes left.....
				while((numpipes + 1) != pipecount)
				{
					//cannot create pipe
					if(pipe(frontpipe) == -1)
					{
						perror("ERROR");
					}

					//fork again to exec each pipe token section
					pid = fork();

					//check if error
					if(pid == -1)
					{
						perror("ERROR");
						exit(EXIT_FAILURE);
					}

					//execute if in child process
					else if(pid == 0)
					{
						int j;
						argcount = 0;
						//continue with "reading" tokens from start towards pipe
						for(j = location[pipecount][0]; j <= location[pipecount][1]; j++)
						{
							//build argument array for execvp
							arg[argcount++] = mytokens[j];
						}

						//combine previous pipe or orevious file descriptors
						dup2(endpipe, STDIN_FILENO);
						//if not last token segment then attach the write end from the previous pipe
						if(pipecount != numpipes)
						{
							dup2(frontpipe[WRITE_END], STDOUT_FILENO);
							close(frontpipe[READ_END]);
						}
						//if last pipe then attach it to file descriptors
						else
						{
							dup2(outputdesc, STDOUT_FILENO);
						}
						//execute the command arguments
						execvp(arg[0], arg);
						perror("ERROR");
						exit(EXIT_FAILURE);
					}
					//if in parent, wait and handle pipes
					else
					{
						waitpid(-1, &executionerr, 0);
						close(frontpipe[WRITE_END]);
						endpipe = frontpipe[READ_END];
						pipecount++;
					}

					}
				}
			
				else if(pidcont != 0)
				{
					if(backgroundflag == FALSE)
					{
						//wait for child to finish if not background
						waitpid(-1, &waiterror, 0);
					}
				}
			}

		return 0;
	}


int main(int argc, char **argv)
{


	//signal(SIGCHLD, SIG_IGN);

		//prompt
		//read line
		//parse line
		//fork (child execs command, parent waits)

		//input = malloc((sizeof(char)*MAX_SIZE));
		//newinput = malloc((sizeof(char)*MAX_SIZE));
		//malloc function
		increasesize();

		//holds the stdin and later sees if NULL or EOF for while loop/parsing
		void *flag;

		strcpy(check, "-n");

		//check to see if there are other arguments past command
		if(argc > 1)
		{
			//check for "-n" if present do not print shell prompt
			if(strcmp(argv[1], check) == 0)
			{
				myprint = "";
			}
		}
		//if there is no "-n" then print prompt
		//print prompt
		shellprompt();

		//read in what the user has submitted (string) until \n or EOF (later used for CTRL+D)
		flag = fgets(input, (sizeof(char)*MAX_SIZE), stdin);

		//while not finished reading input
		while((flag !=  NULL)&&(((int)flag) != EOF))
		{
			//add \0 to end of input just in case
			strcat(input, "\0");

			//parse the following input command
			my_parser();

			//if there is no error in the command line
			if(iserror == FALSE)
			{
				//execute the given command
				execute();
			}

			//malloc again
			increasesize();
			//print shel prompt
			shellprompt();
			//get another input for next loop
			flag = fgets(input, (sizeof(char)*MAX_SIZE), stdin);

		}

	return 0;
}