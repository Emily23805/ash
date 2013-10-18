
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void signal_handler(int sig)
{
	printf("[%d] SIGNAL %d\n", getpid(), sig);

	if (sig == SIGINT)
	{	
		printf("SIGINT - Exiting\n");
		exit(0);
	}
}

int main(int argc, char ** argv)
{
	signal(SIGINT, signal_handler);

	// Sleep forever
	while (1)
	{
		sleep(1);
	}
	return 0;
}