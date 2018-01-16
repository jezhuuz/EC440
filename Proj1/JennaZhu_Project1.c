//E440 Operating Systems Project 1
//Jenna Zhu


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void my_parser(){

while(1){

	char OGcommandline[512] = " ";
	char NEWcommandline[512] = " ";
	//int movcopier = sizeof(char);

	//printf("my_parser>");

	//take in inputs from whatever is submitted into the terminal
	fgets(OGcommandline, 512, stdin);

	//place spaces before and after meta chars while copying over to new array
	int charcount = 0;
	int copycount = 0;

	
	while(OGcommandline[charcount] != '\0'){

		if((OGcommandline[charcount] == '<')||(OGcommandline[charcount] == '|')||(OGcommandline[charcount] == '>')||(OGcommandline[charcount] == '&'))
		{

			NEWcommandline[copycount] = ' ';
			//printf(NEWcommandline);
			//printf("\n");
			NEWcommandline[copycount + 1] = OGcommandline[charcount];
			//printf(NEWcommandline);
			//printf("\n");
			NEWcommandline[copycount + 2] = ' ';
		
		/*	printf(NEWcommandline);
			printf("\n"); */

			copycount = copycount + 3;


		}
		else
		{
			NEWcommandline[copycount] = OGcommandline[charcount];
			//charcount += movcopier;
			copycount++;
		}

		charcount++;

	}

		/*printf(NEWcommandline);
		printf("\n");*/


	char *first = malloc(sizeof(char)*512);

	first = strtok(NEWcommandline, " \n");

	//first token is always a command
	printf("%s - command \n", first);


	char *next = malloc(sizeof(char)*512);
	//next = " ";
	next = strtok(NULL, " \n");

	int metachar = 0;

//loop this til the char array hits \0
	while(next != NULL)
	{

		int tokposition = 1;

		if(*next == '|'){
			tokposition = 0;
			metachar = 1;
			printf("%s - pipe \n", next);
		}

		else if(*next == '>'){
			tokposition = 1;
			metachar = 2;
			printf("%s - output redirect \n", next);
		}

		else if(*next == '<'){
			tokposition = 1;
			metachar = 2;
			printf("%s - input redirect \n", next);
		}	

		else if(*next == '&'){	
			metachar = 1;
			printf("%s - background \n", next);
			next = NULL;
			//return;
		}

		else if ((tokposition == 1)&((metachar == 0)||(metachar == 2))){
				printf("%s - argument \n", next);
			
		}

		else{

			metachar = 0;
			printf("%s - command \n", next);
		}

		next = strtok(NULL, " \n");
	} 

	//return;

}
}

	//if reached the end of the commandline,terminate

	//switch case for rest of the tokens and where they are relative
	//to their position

	//case 0: token is command
	//case 1: token is argument 
	//case 3: token is |
	//case 4: token is <
	//case 5: token is >
	//case 6: token is &

	//switch(tokposition){


	//}

	//if next character is space then last token is finished
	//if next character after a | is not ' ', display | - pipe 
	//////and continue to finish the next token (command) until ' ' or \0
	//if <, input redirect reach next valid character not ' '
	//if >, output redirect reach next valid character not ' '


	//first token is always a command
	//if pervious token is command, current token is an argument



	//keep going until null character \0
	////or if &, it is a background token and at the end of command line

