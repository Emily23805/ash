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

static int background;
static pid_t fg_pid;
static pid_t pid_list[MAX_CHILDS] = {0};

/**
 * Signal handler for catching CTRL-C when a foreground process
 * is running. Notice that we ignore CTRL-C at any other time.
 */
static void sigint_handler(int sig)
{
	kill(fg_pid, SIGKILL);
}

/**
 * Kill all processes running in the backgound.
 */
static void kill_bg()
{
	int i, r, status;
	for (i = 0; i < MAX_CHILDS; i++)
	{	
		if (pid_list[i] > 0)
		{
			r = kill(pid_list[i], SIGKILL);
			if (r < 0)
			{
				printf("Failed killing process %d\n", pid_list[i]);
			}
			else
			{
				// Wait for the process to exit and print info
				waitpid(pid_list[i], &status, 0);
				printf("Killed (by sending SIGKILL) process %d - exited with %d\n", pid_list[i], status);
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

static int remove_from_list(pid_t pid)
{
	int i;
	for (i = 0; i < MAX_CHILDS; i++)
	{
		if (pid_list[i] == pid)
		{
			pid_list[i] = 0;
		}
	}
	return 0;
}

/**
 * Check status of all background processes and print
 * their return value if they have exited.
 */
static int check_bg_status()
{
	int i;
	int status;

	for (i = 0; i < MAX_CHILDS; i++)
	{
		if (pid_list[i] > 0)
		{
			if (waitpid(pid_list[i], &status, WNOHANG) == pid_list[i])
			{
				// Child with PID `pid_list[i]` exited. Remove from list
				printf("Background process %d exited with status %d\n", pid_list[i], status);
				remove_from_list(pid_list[i]);
			}
		}
	}
}


int main(int argc, char* argv)
{
	char buffer[MAX_LEN];	 
	char * list[MAX_LEN];
	struct sigaction action;

	printf(" -- ASH - Awesome Shell -- \n -- Copyright Jacob Pedersen & Andre Christensen 2013\n");

	while(1)
	{
		int i, status;
		char * pch;

		background = 0;

		// Ignore Ctrl-C until a foreground process is started
		action.sa_handler = SIG_IGN;
		sigaction(SIGINT, &action, 0);

		check_bg_status();

		printf(">> ");
		
		// Read input from the prom into a buffer.
		if (gets(buffer)!= NULL) 			
		{   
			// Checks if there is text in the promt.
			if(strlen(buffer) < 1)  			
			{
				// If there is no text in the promt do nothing and get ready for the next command
				continue;					
			}

			i = 0;
			pch = strtok(buffer, " ");	 	//splits the string into tokens. 

			while (pch != NULL)		 		// loops until the last token
			{	
				list[i++] = pch; 			// puts all the tokens into an array  
				pch = strtok(NULL, " "); 	// zeros the pch.
			}	

			if(strcmp(list[i-1], "&") == 0) 	// compair the last torkens in the list with '&' 
			{
				background = 1;				//sets the background flag to '1'
				list[i-1] = 0;				//remove the '&' from the list array again
			}
			else
			{
				list[i] = (char *) 0;	
			}

			//
			// Check for built-in commands.
			//
			if (strcmp(list[0], "exit") == 0)
			{
				exit(0);
			}
			else if (strcmp(list[0], "killbg") == 0)
			{
				kill_bg();
			}
			//
			// Or fork a new child and execute the requested command
			//
			else
			{
				// Fork a new process and get the pid
				pid_t pid = fork();				
				// The child
				if (pid == 0)
				{
					//setpgid(pid, pid);
					// ?? Runs the commands and checks that the exe can be executed if not the execvp will retuen '-1'
					if (execvp(list[0], list) == -1) 
					{
						perror("The following error occurred");	// ?? prints a error massage to the promt.
					}
					exit(0); // ?? the chiled will exit
				} 
				// The parent
				else 
				{
					printf("PID %d started (%s)\n", pid, (background != 1) ? "foreground" : "background");

					// If the process should run in the background
					if(background == 1)
					{
						// TODO Check if this succeds
						add_to_list(pid);

						// Quickly check if the background process has already exited.
						// The WNOHANG tells the system call to return imediately, and is returning
						// the pid of the process if it has exited.
						if (waitpid(pid, &status, WNOHANG) == pid)
						{
							printf("Background process %d exited\n", pid);
							remove_from_list(pid);
						}
						else
						{
							// Background process still running
							// TODO Do something?
						}
					}
					// Or if the process should run in the foreground
					else
					{	
						fg_pid = pid;

						// Catch CTRL-C signals
						action.sa_handler = sigint_handler;
						sigaction(SIGINT, &action, 0);

						// Wait till the process have changed state
						waitpid(fg_pid, &status, 0); 

						// TODO Check what caused the child to exit (exit, signals, etc.)
						printf("PID %d exited with return value: %d\n", pid, status); 
					}
				}
			}
		}
	}	
	return 0;
}



