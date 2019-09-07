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

using namespace std;

int main(){
char * ar[100];
string str="./a.out";
ar[0]=(char*)str.c_str();
	cout<<"a---"<<execvp(ar[0], ar)<<endl;
	
}