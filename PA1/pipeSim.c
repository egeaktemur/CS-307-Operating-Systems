#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	printf("I'm SHELL process, with PID: %d - Main command is: man diff | grep -e -y -A 2\n", getpid());
	int fd[2];
	pipe(fd);
	int rc1 = fork();
	if (rc1 < 0) {
		fprintf(stderr, "fork failed\n");
		exit(1);
	}
	else if (rc1 == 0) {
		printf("I’m MAN process, with PID: %d - My command is: man diff \n", getpid());
		dup2(fd[1], STDOUT_FILENO);
		char *myargs1[3];
		myargs1[0] = strdup("man");	
		myargs1[1] = strdup("diff"); 	
		myargs1[2] = NULL;           	
		execvp(myargs1[0], myargs1);  
	}
	else {
		int rc2 = fork();
		if (rc2 < 0) {
		    	fprintf(stderr, "fork failed\n");
		    	exit(1);
		}
		else if (rc2 == 0) {
			printf("I’m GREP process, with PID: %d - My command is: grep -e -y -A 2 \n", getpid());
			
		    	close(fd[1]);
		    	dup2(fd[0],STDIN_FILENO);
		    	int outputfile=open("./output.txt", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
		    	dup2(outputfile,STDOUT_FILENO);
			char *myargs2[6];
			myargs2[0] = strdup("grep");	
			myargs2[1] = strdup("-e"); 	
			myargs2[2] = strdup("-y");
			myargs2[3] = strdup("-A");	
			myargs2[4] = strdup("2"); 		
			myargs2[5] = NULL;           	
			execvp(myargs2[0], myargs2);  
			close(STDOUT_FILENO);
		}
		else{
			int wc = wait(NULL);
			printf("I’m SHELL process, with PID: %d - execution is completed, you can find the results in output.txt \n", getpid());
		}
	}
	return 0;
}

