#include <stdio.h>
#include <stdlib.h> //exit(), execvp(), EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>
#include <string.h>
#include <sys/wait.h> //waitpid()
#include <unistd.h> //fork(), exec(), pid_t, chdir()
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <bits/stdc++.h>
#include <algorithm>
#include <pwd.h>
using namespace std;

void execOpen(string, string);

void handle_open(char *st){
// int main(){
	// char stre[]="pdef.pdf";
	char stre[100];
	strcpy(stre,st);
	cout<<"received string in handle_open(): "<<stre<<endl;
	char *ar[2];
	char buf[2048];
	string st2;
	string loc="/home/arushi/IIITH/OS/OS_assignment1/temp/"+(string)stre;

	// cout<<"loc: "<<loc<<endl;
	// cout<<"stre "<<stre<<endl;

	ar[0]=strtok(stre,".");//has filename
	// cout<<"filename "<<ar[0]<<endl;
	ar[1]=strtok(NULL,".");//has extension
	
	cout<<"filename "<<ar[0]<<endl;
	cout<<"extension: "<<ar[1]<<endl;

	FILE* fle=fopen("myrc.txt","r");
	if(fle!=NULL){
		cout<<"if"<<endl;
		while(fgets(buf, sizeof(buf), fle)!=NULL){
			char *readlne[10];
			readlne[0]=strtok(buf, "="); //buf of type "mp3=/usr/bin/vlc"
			cout<<"readlne[0] "<<readlne[0]<<endl;
			if(strcmp(readlne[0], ar[1]) == 0){
				readlne[1] = strtok(NULL,"=\n");//get binary path here
				//Delimiter "=\n" taken as each line in buf came appended with newline char
				st2=readlne[1];
				
				// cout<<"st2 "<<st2<<endl;

				/* execute logic */
				execOpen(loc, st2);
				return;
				
			}
		}
	}
	return ;
}

void execOpen(string loctn, string ss){
	// cout<<"here"<<endl;
	// cout<<"ss "<<ss<<endl;

	int status;
	pid_t pid, wpid;
	pid=fork();
	char *bin=(char*)ss.c_str();
	char *loc=(char*)loctn.c_str();

	// cout<<"bin: "<<bin<<endl;
	// cout<<"loc "<<loc<<endl;

	if(pid < 0){//error in forking
		perror("Error in forking:");
	}
	else if(pid == 0){ //child
		if(execl(bin, "xdg-open", loc, (char *)0) ==-1){
			perror("exec failed for child process:");
		
			exit(EXIT_FAILURE); //EXIT_FAILURE: unsuccessful execution of a program
			//the child process exits so that the shell can continue running
		}
		
	}

	else{
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return;
}