// #include "stdio.h"
// #include "unistd.h"

// int main (int argc, const char * argv[])
// {
//     printf("pre-tee\n");

//     if(dup2(fileno(popen("tee out.txt", "w")), STDOUT_FILENO) < 0) {
//         fprintf(stderr, "couldn't redirect output\n");
//         return 1;
//     }

//     printf("post-tee\n");

//     return 0;
// }


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

int main (int argc, const char * argv[])
{
    int fd[2];
	FILE *old_stdout, *new_stdout, *mstdout=stdout;
	FILE *tempfile;


	pipe(fd);
	new_stdout = fdopen(fd[1], "w");
	old_stdout = mstdout;
	mstdout = new_stdout;


	close(fd[1]);
	tempfile=fopen("scrp.txt", "w");

	cout<<old_stdout<<endl;
	cout<<fileno(old_stdout)<<endl;

	dup2(fd[0], fileno(old_stdout));
	dup2(fd[0], fileno(tempfile));



    return 0;
}