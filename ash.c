//
//  
//
//	Copyright Jacob Friis Pdersen & Andre Daniel Christensen DTU 7.okt 2013
//  This is a home made shell with some basic functions.  
//
//---------------------------------------------------------------------------------------------------------------
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>


#define MAX_LEN  255

void endit(int sig);
pid_t pid_list[20];

int main(int argc, char* argv)
{

	char buffer[MAX_LEN];	 
	char* list[MAX_LEN];
	int background = 0;
	

	printf("-- ASH - Awesome Shell --\n-- Copyright Jacob Pedersen & Andre Christensen 2013\n");


	signal(SIGINT, endit);  // signal killes all background processes if Ctrl+c is pressed. 

	while(1)
	{
		background = 0;
		printf(">>");
		
		if (gets(buffer)!= NULL) 			// read input from the prom into a buffer.
		{   
			if(strlen(buffer)<1)  			//checks if there is text in the promt.
			{
				continue;					// if there is no text in the promt do nothing and get ready for the next command
			}

			int i = 0;
			char* pch;
			pch = strtok(buffer, " ");	 	//splits the string into tokens. 

			while (pch != NULL)		 		// loops until the last token
			{	
				list[i++] = pch; 			// puts all the tokens into an array  
				pch = strtok(NULL, " "); 	// zeros the pch.
			}	

			if(strcmp(list[i-1],"&")== 0) 	// compair the last torkens in the list with '&' 
			{
				background = 1;				//sets the background flag to '1'
				list[i-1] = 0;				//remove the '&' from the list array again
			}
			else
			{
				list[i] = (char *)0;		// sets all the elements in the array to zero
			}

			pid_t pid = fork();				//fork a new process and get the pid
			pid_list[0] = pid;

			printf("pid = %d",pid_list[0]);
			if (pid == 0)					// the childe				
			{
				char b[255];	
				
				if (execvp(list[0], list)==-1) // runs the commands and checks that the exe can be executed if not the execvp will retuen '-1'
				{
					perror("The following error occurred");	//prints a error massage to the promt.
				}
				exit(0); // the chiled will exit
			} 
			else   //the parent
			{
				if(background !=1)	// checks if the process should run in 'forground' or 'background'		
				{	
					int status;
					waitpid(pid, &status, 0); //wait till the process have changed state
					printf("%d pid exited with return value: %d\n ",pid, status); // printd the status and the pid to the promt.
				}
			}
		}
	}	
	return 0;
}


void endit(int sig)
{
    printf("\nOPS! - I got the killll signal %d\n", sig);
    
    pid_list[0]=NULL;
    exit(0);
    
}