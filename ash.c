//
//  
//
//	Copyright (C) Jacob Friis Pedersen & Andre Daniel Christensen DTU 7.okt 2013
//  This is a home made shell with some basic functions.  
//
//---------------------------------------------------------------------------------------------------------------
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_CHILDS	20
#define MAX_LEN  255

static pid_t pid_list[MAX_CHILDS] = {0};

static void sigint_handler(int sig)
{
	// Ignore
}

static void kill_bg()
{
	int i, r;
	for (i = 0; i < MAX_CHILDS; i++)
	{	
		if (pid_list[i] > 0)
		{
			r = kill(pid_list[i], SIGINT);
			if (r < 0)
			{
				// ...
			}
			else
			{
				printf("Killed process PID %d\n", pid_list[i]);
			}
			pid_list[i] = 0;
		}
	}
}

static int get_next_avail_index()
{
	int i;
	for (i = 0; i < MAX_CHILDS; i++)
	{
		if (pid_list[i] == 0) 
		{
			return i;
		}
	}
	return -1;
}

static int add_to_list(pid_t pid)
{
	int idx = get_next_avail_index();
	if (idx > -1)
	{
		pid_list[idx] = pid;
	}
	else
	{
		return -1;
	}
}


int main(int argc, char* argv)
{
	char buffer[MAX_LEN];	 
	char* list[MAX_LEN];
	int background = 0;
	
	printf(" -- ASH - Awesome Shell --\n -- Copyright Jacob Pedersen & Andre Christensen 2013\n");

	// Attach a SIGINT handler
	signal(SIGINT, sigint_handler); 

	// Ignore children that exits in the background,
	// and there by avoid them turning into zombies.
	// http://man7.org/linux/man-pages/man2/waitpid.2.html
	signal(SIGCHLD, SIG_IGN);

	while(1)
	{
		background = 0;
		printf(">> ");
		
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

			if (strcmp(list[0], "exit") == 0)
			{
				exit(0);
			}
			else if (strcmp(list[0], "killbg") == 0)
			{
				kill_bg();
			}
			else
			{
				pid_t pid = fork();				//fork a new process and get the pid
				add_to_list(pid);
				if (pid == 0)					// the childe				
				{
					setpgid(pid, pid);
					if (execvp(list[0], list) == -1) // runs the commands and checks that the exe can be executed if not the execvp will retuen '-1'
					{
						perror("The following error occurred");	//prints a error massage to the promt.
					}
					exit(0); // the chiled will exit
				} 
				else   //the parent
				{
					printf("PID %d started (%s)\n", pid, (background != 1) ? "foreground" : "background");

					if(background != 1)	// checks if the process should run in 'forground' or 'background'		
					{	
						int status;
						waitpid(pid, &status, 0); //wait till the process have changed state
						printf("PID %d exited with return value: %d\n", pid, status); // printd the status and the pid to the promt.
					}
				}
			}
		}
	}	
	return 0;
}



