#include <stdio.h>
#include <stdlib.h>	
#include <windows.h>
#include <string.h> //for memset

#define BUFSIZE 256
#define ARGV 9


int execute(int myargc, char *myargv[], int isAmp){
	//from the demo code
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	DWORD i;
	char temp[BUFSIZE];
	memset(temp, 0, BUFSIZE);
	//gets rid of &
	if(isAmp){
		myargv[myargc-1]=0;
		myargc--;
	}
	
	//reassemble the string from my argv
	for(i=0; i<myargc ;i++){
		strcat(temp, myargv[i]);
		strcat(temp, " ");
	}
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof (pi));
	
	if (!CreateProcess(NULL, temp,
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&si,
		&pi))
	{
		fprintf(stderr, "Create Process  Failed\n");
		return -1;
	}else{

	if(!(isAmp)){
		WaitForSingleObject(pi.hProcess, INFINITE);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	return 1;
}
}

int pipe(int myargc, char *myargv[], int indexOfPipe){
	//from the demo code
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	STARTUPINFO si2;
	PROCESS_INFORMATION pi2;
	HANDLE ReadHandle, WriteHandle;
	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	DWORD i;
	
	char temp[BUFSIZE];
	char temp2[BUFSIZE];
	memset(temp, 0, BUFSIZE);
	memset(temp2, 0, BUFSIZE);
	//reassemble the string from my argv
	for(i=0; i<indexOfPipe ;i++){
		strcat(temp, myargv[i]);
		strcat(temp, " ");
	}
	
	for(i=indexOfPipe+1; i<myargc ;i++){
		strcat(temp2, myargv[i]);
		strcat(temp2, " ");
	}

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof (pi));
	ZeroMemory(&si2, sizeof(si2));
	si2.cb = sizeof(si2);
	ZeroMemory(&pi2, sizeof (pi2));

	if (!CreatePipe(&ReadHandle, &WriteHandle, &sa, 0)){
		fprintf(stderr, "Create Pipe Failed\n");
		return 1;
	}

	GetStartupInfo(&si);
	GetStartupInfo(&si2);


	si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	si.hStdInput = ReadHandle;
	si.dwFlags = STARTF_USESTDHANDLES;
	

	if (!CreateProcess(NULL, temp2,
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&si,
		&pi))
	{
		fprintf(stderr, "Create Process 2 Failed\n");
		return -1;
	}

	si2.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	si2.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	si2.hStdOutput = WriteHandle;
	si2.dwFlags = STARTF_USESTDHANDLES;


	if (!CreateProcess(NULL, temp,
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&si2,
		&pi2))
	{
		fprintf(stderr, "Create Process 1 Failed\n");
		return -1;
	}

	WaitForSingleObject(pi2.hProcess, INFINITE);
	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(pi2.hProcess);
	CloseHandle(pi2.hThread);
	return 1;
}


int redirection(int myargc, char *myargv[], int indexOfRe){
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	DWORD i;
	HANDLE fileHandle;
	
	char temp[BUFSIZE];
	memset(temp, 0, BUFSIZE);
		//reassemble the string from my argv
	for(i=0; i<indexOfRe ;i++){
		strcat(temp, myargv[i]);
		strcat(temp, " ");
	}


	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof (pi));

	GetStartupInfo(&si);

	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	si.dwFlags = STARTF_USESTDHANDLES;
	
if(strcmp(myargv[indexOfRe], ">") == 0){
		//opening file if it does not exist then create new file
		if((fileHandle=CreateFile(myargv[indexOfRe+1],GENERIC_WRITE,0,&sa,TRUNCATE_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL )) ==INVALID_HANDLE_VALUE){
			if((fileHandle=CreateFile(myargv[indexOfRe+1],GENERIC_WRITE,0,&sa,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL  )) ==INVALID_HANDLE_VALUE){
				printf("Bad File Open. Error: %x\n", GetLastError());
				return -1;
			}
		}
	
		si.hStdOutput = fileHandle;
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	}else if(strcmp(myargv[indexOfRe], ">>") == 0){
		//opening file if it does not exist then create new file
		if((fileHandle=CreateFile(myargv[indexOfRe+1],GENERIC_WRITE,0,&sa,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL )) ==INVALID_HANDLE_VALUE){
			if((fileHandle=CreateFile(myargv[indexOfRe+1],GENERIC_WRITE,0,&sa,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL )) ==INVALID_HANDLE_VALUE){
				printf("Bad File Open. Error: %x\n", GetLastError());
				return -1;
			}
		}
		si.hStdOutput = fileHandle;
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	}else{
		//opening file
		if((fileHandle=CreateFile(myargv[indexOfRe+1],GENERIC_READ,0,NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL ,NULL )) ==INVALID_HANDLE_VALUE){
			printf("Bad File Open. Error: %x\n", GetLastError());
			return -1;
		}
		si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		si.hStdInput = fileHandle;
	}
	
	if (!CreateProcess(NULL, temp,
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&si,
		&pi))
	{
		fprintf(stderr, "Create Process 2 Failed\n");
		return -1;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	
	return 1;
}


int main(){
	HANDLE StdInHandle =GetStdHandle(STD_INPUT_HANDLE);
	char  buffer[BUFSIZE];
	char *myargv[ARGV];
	char *temp;
	DWORD kindOfCommand =0;
	DWORD myargc = 0;
	DWORD numRead, indexOfSymbol,i;
	
	while(1){
		myargc=0;
		indexOfSymbol=0;
		memset(buffer, 0, BUFSIZE);
		for(i=0; i<ARGV; i++){
			myargv[i]=NULL;
		}
		printf("Myshell>");
		//read from stdin
		if(!(ReadFile(StdInHandle, buffer, BUFSIZE, &numRead, NULL ))){
			printf("Bad Read. Error: %x\n", GetLastError());
			return 2;
		}
		//erase the /n character
		buffer[numRead-2] = 0;
		if(strcmp(buffer, "exit") == 0){
			return 1;
		}
	
		temp = strtok(buffer, " ");
		while(temp !=NULL){
			//put the strings into myargv array and increment myargc
			//if there are special character flag it and the location of the character
			if(strcmp(temp, "|") == 0){
				kindOfCommand=1;
				indexOfSymbol=myargc;
			}else if((strcmp(temp, "<") == 0) || (strcmp(temp, ">>") == 0) || (strcmp(temp, ">") == 0)){
				kindOfCommand=2;
				indexOfSymbol=myargc;
			}else if(strcmp(temp, "&") == 0){
				kindOfCommand=3;
				indexOfSymbol=myargc;
			}
			myargv[myargc] = temp;
			myargc++;
			temp=strtok(NULL, " ");
		}
		//chooses the type of command it is
		switch(kindOfCommand){		
			case 1:
				if(pipe(myargc, myargv, indexOfSymbol)<0){
					return 2;
				}
				break;
			case 2:
				if(redirection(myargc, myargv, indexOfSymbol)<0){
					return 2;
				}
				break;
	
			case 3:
				if(execute(myargc, myargv, indexOfSymbol) < 0){
					return 2;
				}
				break;
			default:
				if(execute(myargc, myargv,indexOfSymbol) < 0){
					return 2;
				}
				break;
		}
		
	}

}
