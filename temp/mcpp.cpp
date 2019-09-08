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

int main(){
	// char *arr[10];
	// arr[0]="/usr/bin/rhythmbox";
	string path1="/home/arushi/IIITH/OS/OS_assignment1/temp/music.mp3";
	string path4="/home/arushi/IIITH/OS/video.mp4";
	string path2="/home/arushi/IIITH/OS/text.txt";
	string path3="/home/arushi/IIITH/OS/pdef.pdf";
	char *media, *ascii, *docViewer;
	media="/usr/bin/vlc";
	ascii="/usr/bin/subl";
	docViewer="/usr/bin/evince";

	pid_t pid, wpid;
	int status;

	/* execute logic */
	pid=fork();

	// cout<<"pid "<<pid<<endl;

	if(pid < 0){//error in forking
		perror("Error in forking:");
	}

	else if(pid == 0){ //child
			if(execl(media, "xdg-open", path1.c_str(), (char *)0) ==-1){
				// cout<<"execvp err"<<endl;
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

	

	

	return 0;

	// execvp()
}