#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

int main( int argc, char **argv ) {
	pid_t pid = fork();
	if ( pid == 0 ) {
		execvp( "./sequential_min_max", argv);
		return 0;
	} else {
    int status;
	wait(&status);
	}
    
	

	return 0;
}