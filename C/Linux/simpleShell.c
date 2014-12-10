#include <stdio.h>
#include <string.h> //for fgets() and feof()
#include <stdlib.h>	//exit()
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFSIZE 256
#define ARGVSIZE 10
void selectCommand(int myargc, char* myargv[]);

void execute(int   argc, char *argv[], int isAmp) {	
	//this part of the code is copied from the demo code. I have deleted the printf statements and add isAmp to check for the &.
	int	pid;
	if(isAmp){
		argv[argc-1]=0;
	}
	if ((pid = fork()) == -1 ) { /* error exit - fork failed */ 
		perror("Fork failed");
		exit(-1);
	}
	if (pid == 0) { /* this is the child */
		//I changed the execl to execvp
		execvp(argv[0],&argv[0]);
		/* error exit - exec returned */
		perror("Exec returned");
		exit(-1);
	} 
	if(!(isAmp)){ /* this is the parent -- wait for child to terminate */
		wait(pid,0,0);
	}
}
void pippingCommand(int   argc, char *argv[], int locationOfSymbol) {	
	int	pid1,pid2;
	int	pipeID[2];
	int	status;
	argv[locationOfSymbol]=NULL;
	//this part of the code is from the demo code. I deleted the printf statements and used dup2 instead of dup
	if ((status = pipe(pipeID)) == -1) { /* error exit - bad pipe */
		perror("Bad pipe");
		exit(-1);
	}
	if ((pid1 = fork()) == -1) {  /* error exit - bad fork */
		perror("Bad fork");
		exit(-1);
	}
	if (pid1 == 0) { 
		/* The first child process */
		dup2(pipeID[0],0);
		close (pipeID[0]);
		close(pipeID[1]);
		//execute the command right of the |
		execvp(argv[locationOfSymbol+1],&argv[locationOfSymbol+1]);
		
		/* error exit - exec returned */
		perror("Execl returned");
		exit(-1);
	}
	/* this is the parent again */
	if ((pid2 = fork()) == -1) { /* error exit - bad fork */
		perror("Bad fork");
		exit(-1);
	}
	if (pid2 == 0) { /* the second child process */
		dup2(pipeID[1],1);
		close(pipeID[0]);
		close(pipeID[1]);
		//execute the command right of |
		execvp(argv[0],&argv[0]);
		
		/* error exit - exec returned */
		perror("Execl returned");
		exit(-1);
	}
	/* back to the parent again */
	close(pipeID[0]);
	close(pipeID[1]);
	wait(pid1,0,0);
	wait(pid2,0,0);
}

void redirection(int argc, char* argv[], int locationOfSymbol){
	int	pid1;
	int fileHandle;
	
	if ((pid1 = fork()) == -1) {  /* error exit - bad fork */
		perror("Bad fork");
		exit(-1);
	}
	
	if (pid1 == 0) { 
		/* The first child process */
		//checks if its < , then it will open the file and read from the file only. if the file doesn't exist, exit program
		if((strncmp(argv[locationOfSymbol],"<",1)) == 0){
				if(fileHandle=open(argv[locationOfSymbol+1],  O_RDONLY) < 0){
					perror("Failed to open file");
					exit(-1);
				
				}
			//the STDIN is now connected to the STDOUT of the file
				if(dup2(fileHandle, STDIN_FILENO) < 0){
					perror("Redirecting Failed");
					exit(-1);
				}
		//checks if its > or >>, if its > then when openning the file, it will erase the file first. If its >>
		// then it will just append to the end of the file
		}else if((strncmp(argv[locationOfSymbol],">",1)) == 0){
			if((strncmp(argv[locationOfSymbol],">>",2))== 0){
				fileHandle=open(argv[locationOfSymbol+1], O_APPEND | O_WRONLY | O_CREAT, S_IWRITE | S_IREAD);
			}else{
				fileHandle=open(argv[locationOfSymbol+1], O_TRUNC | O_WRONLY | O_CREAT, S_IWRITE | S_IREAD);
			}
			//the STDOUT is connected to the STDIN of the file
			if(dup2(fileHandle, 1) < 0){
			perror("Redirecting Failed");
			exit(-1);
			}
		}		
		argv[locationOfSymbol] = NULL;
		//close(fileHandle);
		//execute the command to the right of the symbol
		execvp(argv[0],&argv[0]);
		
		/* error exit - exec returned */
		perror("Execl returned");
		exit(-1);
	}
	/* back to the parent again */
	wait(pid1,0,0);
}
/*
* reads the line from STDIN and put the string into the buffer. The function returns -1 if it detects EOF and
* 1 if it is succesful.
*/
int getCommandLine(char *buffer){
	int strLength;
	//clear buffer
	memset(buffer, 0, sizeof(buffer));
	//returns -1 when it detects EOF
	if(feof(stdin)){
			return -1;
	}else{
			//gets the command line and store it in the buffer and return 1
			fgets(buffer, BUFSIZE, stdin);
			strLength=strlen(buffer);
			//delete the '\n' character
			buffer[strLength-1]=0;
	}
			return 1;
	}

/**
 * parses the string in the buffer and put each string token into the argv array.
 * returns the total string token in argv array or -1 if the exit command is entered
 */
int parse(char* buffer, char *argv[]){
	char *temp;
	int argc=0;
	int i;
	//clear the argv array
	for(i=0; i<ARGVSIZE; i++){
		argv[i]=NULL;
	}
	
	//tokenize the strings in the buffer that is seperated by whitespace
	temp= strtok(buffer, " ");
	//returns -1 if it detects the exit command
	if((strncmp(temp,"exit",4)) == 0){
			return -1;
	}
	//while there is still string tokens in the buffer
	while(temp !=NULL){
		//put the strings into argv array and increment argc
		argv[argc] = temp;
		argc++;
		temp=strtok(NULL, " ");
	}
	
	//return the number of strings added into argv
	return argc;
}

/*
 * goes through myargv array to see if any special commands is entered (eg. > >> < << | &) and 
 * calls the right functions to execute the command.
*/
void selectCommand(int myargc, char* myargv[]){
		int i;
		int symbol = 0;
		
		//going through the myargv array
		for(i=0;i<myargc;i++){
			if((strncmp(myargv[i],"|",1)) == 0){
				//if it's '|' then call pippingCommand
				symbol = 1;
				break;
			}else if( ((strncmp(myargv[i],"<",1)) == 0) || ((strncmp(myargv[i],">",1)) == 0) ){
				//if it's > >> < << then call redirection function
				symbol = 2;
				break;		
			}else if((strncmp(myargv[i], "&" , 1)) ==0){
				symbol = 3;
				break;
			}
		}
	
		switch(symbol){
			case 1: 
				pippingCommand(myargc,myargv,i);
				break;
			case 2:
				redirection(myargc,myargv,i);
				break;
			case 3:
				execute(myargc, myargv,1);
				break;
			default:
				execute(myargc, myargv,0);
		}
}


int main(){
	//buffer to hold the STDIN string
	char buffer[BUFSIZE];
	//contain the parsed command line
	char *myargv[ARGVSIZE];
	//the count of the parsed command line
	int myargc;
	//loops until the EOF or "exit" command is entered
	while(1){
		printf("Myshell>");
		myargc=0;
		//get the command. if the function returns -1, the EOF is entered so end program
		if((getCommandLine(buffer)) < 0){
			exit(-1);
		}
		//parses the command line. If function returns -1, the exit command is entered so end program, otherwise it returns the number 
		//of parsed strings
		if((myargc=parse(buffer, myargv))< 0){
			exit(-1);
		}
		//execute command
		selectCommand(myargc, myargv);
		}
		

};