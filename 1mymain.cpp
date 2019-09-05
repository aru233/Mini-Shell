#include <stdio.h>
#include <stdlib.h> //exit(), execvp(), EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>
#include <string.h>
#include <sys/wait.h> //waitpid()
#include <unistd.h> //fork(), exec(), pid_t, chdir()

using namespace std;

#define BUFF_SIZE 1024

void parse_input(char* , char **);
void execute_cmd(char **);
int sizeofinp(char **);
void handle_redir(char **);

int main(){
	

	while(1){
		cout<<"$";
		//char input_cmd[BUFF_SIZE][BUFF_SIZE];
		char* input_cmd[2048];
		string str;
		getline(cin,str);
		if(str.empty()){ // Needed for ctrl+D case...as prog was entering infine loop coz of that
			cout<<endl;
			break;
		}

		char arr[str.length()+1];
		str.copy(arr,str.length());
		arr[str.length()]='\0';
		parse_input(arr,input_cmd);

		
		int len = sizeofinp(input_cmd);
		// cout<<"len "<<len<<endl;
		
		if(strcmp(input_cmd[0],"exit")==0){
			exit(0);
		}

		if(strcmp(input_cmd[0],"cd")==0){				
			if(len==1 || string(input_cmd[1]).empty()){
				errno=ENOENT;
				perror("Enter a valid filename:");
				continue;
			}
			string filename = input_cmd[1];
			chdir(filename.c_str());
			continue;
		}

		if(len >= 3){

		}
		
		
 
  
		// cout<<"1st---"<<input_cmd[0]<<endl;
		// cout<<"2nd--"<<input_cmd[1]<<endl;

		execute_cmd(input_cmd);

		cout<<"back in main"<<endl;
		

	}//end of outermost while()
}

void handle_redir(char** input_cmd){

}

void parse_input(char* arr, char** input_cmd){
	while(*arr != '\0')
	{
		while(*arr==' ' || *arr == '\t' || *arr == '\n')
		{
			while(*arr==' ' || *arr == '\t'){
				arr++;
			}
			*(arr-1)='\0';
			arr++;
			
		}
		
		*(input_cmd++) = arr;
		
		
		while(*arr != ' ' && *arr != '\t' && *arr != '\n' && *arr != '\0')
		{
			arr++;
		}

	}
	*input_cmd=NULL;

	// int i=0;
	
	// int pos=0;
	// string word="";
	// char l[40];
	// while(1){
	// 	char c;
	// 	c=getchar();
	// 	cout<<"char "<<c<<" "<<endl;

	// 	//TODO: handle quotes, backslash etc like:
	// 	// echo "hi, there" shouldnt split "hi," and "there" into 2 diff words

	// 	if(c==' ' || c=='\t' || c=='\n'){
	// 		if(c==' ' && word=="" && pos==0){ //ignoring leading spaces in the command
	// 			continue;
	// 		}
	// 		// cout<<"gfd"<<endl;
	// 		//strcpy(input_cmd[pos],word.c_str());
	// 		// strcpy(l,word.c_str());
	// 		//input_cmd[pos]=&(*str.begin());
	// 		int j;
	// 		char arr[(word.length())+1];
	// 		for(j=0;j<word.length();j++)
	// 		{
	// 			input_cmd[pos][j]=word[j];
	// 		}
	// 		input_cmd[pos][j]='\0';

	// 		cout<<"word "<<word<<endl;
	// 		cout<<"inp cmd "<<pos<<" "<<input_cmd[pos]<<endl;
	// 		word="";

			

	// 		pos++;
	// 		if(c=='\n') break;
	// 		// cout<<"inp cmd outside"<<pos-1<<" "<<input_cmd[pos-1]<<endl;		
	// 	}
	// 	else{
	// 		word+=c;
	// 	}
	// }//end of "reading input string" while()

	// cout<<"pos after "<<pos<<endl;
	// input_cmd[pos][0]='\0'; //A NULL pointer is used to mark the end of the array

	// cout<<"parse "<<input_cmd[0]<<endl;
	// cout<<"parse "<<input_cmd[1]<<endl;
	
		
}

int sizeofinp(char **input_cmd){
	int i=0;
	while(input_cmd[i]){
		i++;
	}
	return i;
}

void execute_cmd(char **input_cmd){
	cout<<"in exec cmd...HI!!!"<<endl;

	// cout<<input_cmd[0]<<endl;
	// cout<<input_cmd[1]<<endl;

	pid_t pid, wpid;
	int status;

	/* execute logic */
	pid=fork();

	cout<<"pid "<<pid<<endl;

	if(pid < 0){//error in forking
		perror("Error in forking:");
	}

	else if(pid == 0){ //child
		cout<<"In child "<<endl;
		// char **argv;
		// argv=input_cmd;
		if(execvp(*input_cmd, input_cmd) ==-1){
			cout<<"execvp err"<<endl;
			perror("exec failed for child process:");
		//A successful call to execvp does not have a return value .
		//However, a -1 is returned if the call to execvp is unsuccessful.
		
			exit(EXIT_FAILURE); //EXIT_FAILURE: unsuccessful execution of a program
		//the child process exits so that the shell can continue running
		}
	}

	else{ //parent 
		cout<<"In parent "<<endl;
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	// return 1;
	return;
}
