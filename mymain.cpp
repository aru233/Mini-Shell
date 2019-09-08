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

#include "open.h"

#define BUFF_SIZE 1024

void setup_myrc();
void init_env_var();
string getPATH();
void parse_input(char* , char **);
void execute_cmd(char **, int);
int sizeofinp(char **);
int handle_redir(int, int, int ,char **, int);
void handle_cd(int , char**);
int findPosOfAmp(char** , char , int);
int findPosOf(char** , string);
int findPosOf1(string , char);
void setInMap(string , string );
string checkInMap(string );
void parseForAlias(char *);
void handle_echo(string);
void handle_pipes(int , string);


map<string, string> mp_alias;
map<string, string> mp_env;
char currDir[2048];

int main(){
	int background, aliasflag=0;
	string y;
	int pos;
	setup_myrc();

	while(1){
		getcwd(currDir, 2048);
		// cout<<currDir<<"$";
		cout<<mp_env["USER"]<<"@"<<mp_env["HOSTNAME"]<<":"<<currDir<<mp_env["PS1"]<<" ";

		background=0;
		// aliasflag=0;
		char *input_cmd[2048];
		string str;
		getline(cin,str);
		if(str.empty()){ // Needed for ctrl+D case...as prog was entering infine loop coz of that
			cout<<endl;
			break;
		}
		char arr[str.length()+1];

		str.copy(arr,str.length());
		arr[str.length()]='\0';

		parse_input(arr, input_cmd);
		
		int len = sizeofinp(input_cmd);
		cout<<"len "<<len<<endl;

		if(strcmp(input_cmd[0],"exit")==0){
			exit(0);
		}

		// if(len == 1 && strcmp(input_cmd[0], "&")){
		// 	perror("bash: syntax error near unexpected token `&'");
		// 	continue;
		// }


		if(strcmp(input_cmd[0], "alias")==0){
			cout<<"main: in alias if"<<endl;
			string ali;
			ali=str.substr(6);
			cout<<"ali "<<ali<<endl;
			parseForAlias((char*)ali.c_str());
			continue;
		}

		if(strcmp(input_cmd[0], "open")==0){
			cout<<"main: in open if"<<endl;
			handle_open(input_cmd[1]);
			continue;
		}


		//alias x="ls"
		//x -l -a
		if((y=checkInMap(string(input_cmd[0]))) != "-1"){
			cout<<"alias se new str "<<y<<endl;
			// aliasflag=1;
			//replace the first word with alias, if exists; 
			// not checking for alias for other words
			len=sizeofinp(input_cmd);
			string r=string(input_cmd[0]), cmd=y;
			len=sizeofinp(input_cmd);
			int ll=r.length();
			char arr1[100];

			for(int i=1;i<len;i++){
				cmd=cmd+" "+string(input_cmd[i]);
			}
			cout <<"cmd "<<cmd<<endl;

			// char arr1[str.length()+1];
			strcpy(arr1,cmd.c_str());
			arr1[cmd.length()]='\0';
			parse_input(arr1, input_cmd);
			// cout<<"yo"<<endl;
			len=sizeofinp(input_cmd);
		}
		// if(aliasflag==0){
		// 	parse_input(arr, input_cmd);
		// 	len=sizeofinp(input_cmd);
		// }

		// cout<<"len after "<<len<<endl;
		// cout<<input_cmd[0]<<endl;
		// cout<<input_cmd[1]<<endl;

		// /* background proc */
		// if(!strcmp(input_cmd[len-1],"&")){// for ls &
		// 	input_cmd[len-1]=NULL;
		// 	len--;
		// 	background=1;
		// 	cout<<" in & ___background "<<background<<endl;
		// 	sizeofinp(input_cmd);
		// }

		// background = findPosOfAmp(input_cmd, '&', len);
		// cout<<" else, background "<<background<<endl;
		// /* --- background proc */
		

		if(strcmp(input_cmd[0],"cd")==0){
			// sizeofinp(input_cmd);
			handle_cd(len, input_cmd);
			continue; //no need to execute execvp() for cd;	
		}

		/* pipes */
		pos=findPosOf1(str, '|');
		if(pos!=-1){
			cout<<"FOund pipe .. in main"<<endl;
			handle_pipes(len, str);

			cout<<"back in main after handle pipes"<<endl;
			continue;
		}
		/*---pipes--- */

		/* Redirection */
		pos=findPosOf(input_cmd, ">");
		// cout<<"pos1 "<<pos<<endl;
		if(pos!=-1){//a redirection case
			if(!handle_redir(len, pos, 1, input_cmd, background)){//1: ">"
				continue;//invalid filename prob, so we continue;
			}
			continue; // No need to execute execute_cmd() as it's already handled by handle_redir()
		}

		pos=findPosOf(input_cmd, ">>");
		// cout<<"pos2 "<<pos<<endl;
		if(pos!=-1){//a redirection case
			if(!handle_redir(len, pos, 2, input_cmd, background)){//2: ">>"
				continue;//invalid filename prob, so we continue;
			}
			continue; // No need to execute execute_cmd() as it's already handled by handle_redir()
		}
		/* ...Redirection End...*/
		
  		/* echo */		
		if(str.length()>=4 && str.substr(0,4)=="echo"){
			handle_echo(str);
			continue;
		}
		/* ..echo end.. */

		execute_cmd(input_cmd, background);

		cout<<"back in main"<<endl;
		

	}//end of outermost while()
}

void setup_myrc(){

	init_env_var();

	FILE* fle=fopen("myrc.txt","w");
	if(fle!=NULL){
		// cout<<"gtrearr"<<endl;
		// fputs("HOME=\n",fle);
		// fprintf(fle, "gcvxz");
		fprintf(fle, "HOME=%s\n", mp_env["HOME"].c_str());
		fprintf(fle, "USER=%s\n", mp_env["USER"].c_str());
		fprintf(fle, "HOSTNAME=%s\n", mp_env["HOSTNAME"].c_str());
		fprintf(fle, "PS1=%s\n", mp_env["PS1"].c_str());
		fprintf(fle, "PATH=%s\n", mp_env["PATH"].c_str());

		fprintf(fle, "mp4=%s\n", "/usr/bin/vlc");
		fprintf(fle, "mp3=%s\n", "/usr/bin/vlc");
		fprintf(fle, "pdf=%s\n", "/usr/bin/evince");
		fprintf(fle, "txt=%s\n", "/usr/bin/subl");
		fprintf(fle, "cpp=%s\n", "/usr/bin/subl");

		fclose(fle);
	}
	else{
		perror("Can't open file");
	}

}

void init_env_var(){
	// cout<<"here in init_env_var"<<endl;

	struct passwd *pwd=getpwuid(getuid());
	char *HOME = pwd->pw_dir;
	char *USER = pwd->pw_name;
	char HOSTNAME[1028];
	string PATH;
	string PS1;
	gethostname(HOSTNAME, 1028);
	if(geteuid()==0){
		// PS1=string(USER)+"@"+string(HOSTNAME)+"#";
		PS1="#";
	}
	else{
		PS1="$";
	}
	// cout<<"kiu1"<<getuid()<<endl;

	// 	cout<<HOME<<endl;
	// cout<<USER<<endl;
	// cout<<HOSTNAME<<endl;
	// cout<<PS1<<endl;
	// cout<<mp_env["HOME"]<<endl;

	PATH=getPATH();
	// cout<<"PAth "<< PATH<<endl;

	mp_env["PATH"]=PATH;
	mp_env["HOME"]=string(HOME);
	mp_env["PATH"]=PATH;
	mp_env["USER"]=string(USER);
	mp_env["HOSTNAME"]=string(HOSTNAME);
	mp_env["PS1"]=PS1;

}

string getPATH(){
	// string p="/etc/manpath.config";
	// char *mpath;
	string mpath="";
	char buf[2048];
	int n=1;
	FILE *mf = fopen("/etc/manpath.config","r");
	if(mf!=NULL){
		// cout<<"mf not null "<<endl;
		while(fgets(buf, sizeof(buf), mf)!=NULL){
			char *path[1024];
			path[0] = strtok(buf, "\t ");
			if(strcmp(path[0], "MANPATH_MAP") == 0){
				path[1] = strtok(NULL,"\t ");
				string s=path[1];
				mpath+=s;
				mpath+=":";
			}	
		}
	}
	
	fclose(mf);

	return mpath.substr(0, mpath.length()-1);	
}

void setInMap(string k, string v){
	mp_alias[k]=v;
}

string checkInMap(string s){
	if(mp_alias.find(s)!=mp_alias.end()){
		return mp_alias[s];
	}
	return "-1";
}

void parseForAlias(char *stre){
	int i=0;
	string s=string(stre), key="", val="", mval;
	int l=s.length();
	cout<<"s "<<s<<endl;
	char *ar[10];

	ar[0] = strtok(stre,"=");
	ar[1] = strtok(NULL,"=");
	cout<<"key "<<ar[0]<<endl;
	cout<<"val"<<ar[1]<<endl;

	mval=string(ar[1]);

	mval.erase(remove(mval.begin(), mval.end(), '\"' ),mval.end());
	
	cout<<"key "<<ar[0]<<endl;
	cout<<"val "<<mval<<endl;

	setInMap(string(ar[0]), mval);	
}

int findPosOf(char** input_cmd, string str){
	int i=0;
	while(input_cmd[i]){
		if(strcmp(input_cmd[i], str.c_str())==0){
			return i;
		}
		i++;
	}
	return -1;
}

int findPosOf1(string mystr, char str){
	// int i=0;
	for(int i=0;i<mystr.length();i++){
		if(mystr[i]==str){
			return i;
		}
	}
	return -1;
}

/*This function will
1- check if & present at the end of command
2- If present, removes it from the command by setting that pos NULL
3- and returns 1 to set the flag "background"
*/
int findPosOfAmp(char** input_cmd, char sym, int len){
	string str=input_cmd[len-1];
	int len1=str.length();
	if(str[len1-1] == sym){
		input_cmd[len-1][len1-1]='\0';
		return 1;
	}	
	return 0;
}

void count_pipes(string str, char** pipcmd, int* pipcomcount){
	cout<<"Here in count_pipes() "<<endl;
	*pipcomcount=1;
	pipcmd[0]=strtok((char*)str.c_str(), "|");

	cout << "pipcmd[0]: "<<pipcmd[0]<<endl;
	while((pipcmd[*pipcomcount] = strtok(NULL, "|")) != NULL){
		cout<<"pipcomcount "<<*pipcomcount<<", pipcmd[*pipcomcount] "<<pipcmd[*pipcomcount]<<endl;
		(*pipcomcount)++;
		// cout<<"pipcomcount "<<*pipcomcount<<endl;
		// cout<<"pipcomcount "<<*pipcomcount<<", pipcmd[*pipcomcount] "<<pipcmd[*pipcomcount]<<endl;
	}
	pipcmd[*pipcomcount]=NULL;
}

void handle_pipes(int leng, string str){
	int fd[2];
	int fdpos;
	int pt;
	pid_t pid;
	char* pipcmd[1024];
	int pipcomcount=1, redir_flag=0;
	// count_pipes(str, pipcmd, &pipcomcount);

	pipcmd[0]=strtok((char*)str.c_str(), "|");

	// cout << "pipcomcount 0 "<<"pipcmd[0]: "<<pipcmd[0]<<endl;
	while((pipcmd[pipcomcount] = strtok(NULL, "|")) != NULL){
		// cout<<"pipcomcount "<<pipcomcount<<", pipcmd[pipcomcount] "<<pipcmd[pipcomcount]<<endl;
		pipcomcount++;
		// cout<<"pipcomcount "<<*pipcomcount<<endl;
		// cout<<"pipcomcount "<<*pipcomcount<<", pipcmd[*pipcomcount] "<<pipcmd[*pipcomcount]<<endl;
	}
	pipcmd[pipcomcount]=NULL;


	// cout<<"size of pipcmd "<<sizeofinp(pipcmd)<<endl;

	// cout << "pipcomcount .. in handle_pipes " << pipcomcount<<endl;
	// cout<<"pipcmd[0] "<< pipcmd[0]<<endl;
	// cout<<"pipcmd[1] "<< pipcmd[1]<<endl;
	pt=-1;
	fdpos=0;

	while(pt++<pipcomcount){
		pipe(fd);
		pid=fork();
		
		if(pid==-1){
			perror("Error in forking:");
		}
		else if(pid==0){//child
			dup2(fdpos, STDIN_FILENO);
			//for the 1st function, this won't change anything as 0,0 but for further calls, the input will be taken from fdpos 
			// //which was written to by prev command
			// cout<<pt<<endl;
			char *ark[100];
			char coms[100];
			strcpy(coms, pipcmd[pt]);
			// cout<<"b4: pt "<<pt<<" pipcmd[pt] "<<pipcmd[pt]<<endl;
			string parsedCopy[100];
			for(int i=0; pipcmd[i]!=NULL; i++)
			{
				string cp=string(pipcmd[i]);
				parsedCopy[i]=cp;
			}
			parse_input(coms, ark);
			// cout<<"pipcmd[0] "<< parsedCopy[0]<<endl;
			// cout<<"pipcmd[1] "<< parsedCopy[1]<<endl;
			cout<<"arr[0] "<< ark[0]<<endl;
			cout<<"arr[1] "<< ark[1]<<endl;

			if(pt<pipcomcount-1){//for last command, o/p to be written to terminal only
				dup2(fd[1], STDOUT_FILENO);
			}
			close(fd[0]);
			// cout<<"in handle_pipes() not redir case: "<<arr[0]<<endl;if(pt==pipcomcount-1){//checking redirection in last cmd
			
			if(pt==pipcomcount-1){//checking redirection in last cmd
				cout<<"pipcom-1 mai pt:"<<pt<<endl;
				/* Redirection */
				int pos, background=0;
				int len = sizeofinp(ark);;
				pos=findPosOf(ark, ">");
				cout<<"pos of > in.. "<<pos<<endl;
				// cout<<"pos1 "<<pos<<endl;
				if(pos!=-1){//a redirection case
					cout<<"redir mai ghusa 1"<<endl;
					handle_redir(len, pos, 1, ark, background);//1: ">"
					// if(!handle_redir(len, pos, 1, pipcmd, background)){//1: ">"
					// 	continue;//invalid filename prob, so we continue;
					// }
					// continue; // No need to execute execute_cmd() as it's already handled by handle_redir()
					cout<< "came back after handling redir"<<endl;
					redir_flag=1;
				}

				pos=findPosOf(ark, ">>");
				// cout<<"pos2 "<<pos<<endl;
				if(pos!=-1){//a redirection case
					cout<<"redir mai ghusa 2"<<endl;
					handle_redir(len, pos, 2, ark, background);//2: ">>"
					// if(!handle_redir(len, pos, 2, input_cmd, background)){//2: ">>"
					// 	continue;//invalid filename prob, so we continue;
					// }
					// continue; // No need to execute execute_cmd() as it's already handled by handle_redir()
					redir_flag=1;
				}
				/* ...Redirection End...*/
			}
			else if(pt!=pipcomcount-1 || redir_flag==0){
				if(execvp(ark[0], ark) ==-1){
					perror("exec failed for child process:");
					//A successful call to execvp does not have a return value .
					//However, a -1 is returned if the call to execvp is unsuccessful.
				
					exit(EXIT_FAILURE); //EXIT_FAILURE: unsuccessful execution of a program
					//the child process exits so that the shell can continue running
			
				}	
			}						
			
		}
		else{//parent
			cout<<"in handle_pipe parent"<<endl;
			// pt++;
			waitpid(-1, NULL, 0);
			close(fd[1]);// after writing to the write end of the pipe, we close it
			//and will read from the read end of the pipe
			fdpos=fd[0];//taking input from read input for the next command in pipeline
			
		}
	}
}

void handle_echo(string str){
	/*cases need to be handled:
	echo "$PATH"lfk
	echo     ghjk
	echo "$PATH" 
	echo 'kjn'jd'
	echo "hjkd's"
	    echo "dshg"
	*/

	cout<<"str in echo "<<str<<endl;
	int ech_l1=str.length(), ech_l2;
	string str1;
	str.erase(remove( str.begin(), str.end(), '\"' ),str.end());
	if(str[5]=='\''){
		str.erase(remove( str.begin(), str.end(), '\'' ),str.end());
	}
	cout<<"str in echo ..after "<<str<<endl;

	ech_l2=str.length();
	if(ech_l1-ech_l2==1){//Quote mismatch
		cout<<"QUOTE mismatch"<<endl;
		return;
	}

	int i=5;
	while(str[i]!='$' && i<ech_l2){
		i++;
	}
	if(str[i]=='$'){
		str1=str.substr(i+1);
		if(str1=="$"){
			cout<<getpid()<<endl;
		}
		else if(str1=="PATH"){
			cout<<mp_env[str1]<<endl;
			// cout<<mp_env[str1]<<endl;
		}
		else if(str1=="HOME"){
			cout<<mp_env[str1]<<endl;
		}
		else if(str1=="USER"){
			cout<<mp_env[str1]<<endl;
		}
		else if(str1=="HOSTNAME"){
			cout<<mp_env[str1]<<endl;
		}
		else if(str1=="PS1"){
			cout<<mp_env[str1]<<endl;
		}
		else{
			cout<<str.substr(5)<<endl;
			return;
		}
	}
	else{
		cout<<str.substr(5)<<endl;
		return;
	}
}

void handle_cd(int len, char** input_cmd){
	cout<<"here in cd"<<endl;
	sizeofinp(input_cmd);
	// if(len==1 || string(input_cmd[1]).empty()){
	// 	errno=ENOENT;
	// 	perror("Enter a valid filename:");
	// 	return;
	// 	// continue;
	// }

	string filename;
	string ss=mp_env["HOME"];
	char home[1024]; 
	strcpy(home, ss.c_str());

	// cout<<"ss"<<ss<<endl;
	// char home[1024]="/home/arushi";
	int x;


	if(len==1 || string(input_cmd[1]).empty() ){// writing just cdcd takes us to home dir
		cout<<"cd: mai idhar hi gus gaya "<<endl;
		sprintf(currDir, "%s", home);

		// getcwd(currDir, 2048);
		// cout<<"currDir "<<currDir<<endl;
		if(chdir(currDir)!=0){
			perror("Error in changin dir");
		}
		return;
	}

	filename = input_cmd[1];
	cout<<"cd: filename "<<filename<<endl;
	
	// cout<<"len in cd "<<endl;
	
	
	// str path;
	if(filename[0] == '~'){ // cd ~/IIITH types should also work
		sprintf(currDir, "%s%s", home, input_cmd[1]+1);

		// getcwd(currDir, 2048);
		// cout<<"currDir "<<currDir<<endl;
		if(chdir(currDir)!=0){
			perror("Error in changin dir");
		}

	}
	
	else if((x=chdir(filename.c_str()))!=0){
		cout<<"x "<<x<<endl;
		perror("Error in changin dir");
	}
	// continue;
}

int handle_redir(int len, int pos, int flag, char** input_cmd, int background){
	if(string(input_cmd[pos+1]).empty()){
		errno=ENOENT;
		perror("Enter a valid filename:");
		return 0;
	}
	pid_t pid, wpid;
	int status, fd1;
	pid=fork();

	if(pid < 0){//error in forking
		perror("Error in forking:");
	}
	else if(pid == 0){ //child
		if(flag==1){

			cout<< "Here for >"<<endl;
			if((fd1=open(input_cmd[pos+1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR|S_IWUSR)) == -1){
				perror("Error in opening file");
			}
		}
		if(flag==2){

			cout<< "Here for >2 "<<endl;
			if((fd1=open(input_cmd[pos+1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR|S_IWUSR)) == -1){
				perror("Error in opening file");
			}
		}
		
		dup2(fd1, STDOUT_FILENO);
		input_cmd[pos]=NULL;

		// cout<<"bhai "<<input_cmd[0]<<endl;
		// if(strcmp(input_cmd[0],"echo")==0){
		// 	cout<<"echo hai"<<endl;
		// 	size_t op_pos = flag==1 ? str.find(">") : str.find(">>"); 
  //   		if (op_pos != string::npos){
  //   			str=str.substr(0,op_pos);
  //   			handle_echo(str);
  //   		}
  //   		cout<<"i am out"<<endl;
  //   		close(fd1);
			
		// }

		if(execvp(input_cmd[0], input_cmd) ==-1){
			// cout<<"execvp err"<<endl;
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
	cout<<"abt to return"<<endl;
	return 1;
}

void parse_input(char* arr, char** input_cmd){
	// cout<<"parse_input "<<arr<<endl;
	while(*arr != '\0')
	{
		while(*arr==' ' || *arr == '\t' || *arr == '\n')
		{
			*arr++='\0';
			while(*arr==' ' || *arr == '\t'){
				arr++;
			}
			
		}
		if(*arr != '\0'){
			*input_cmd++=arr;
		}
			
		
		while(*arr != ' ' && *arr != '\t' && *arr != '\n' && *arr != '\0')
		{
			arr++;
		}

	}
	*input_cmd=NULL;	
	// cout<<"arr* "<<arr<<endl;
}

int sizeofinp(char **input_cmd){
	int i=0;
	while(input_cmd[i]){
		cout<<"i "<<i<<"  "<<input_cmd[i]<<endl;
		i++;
	}
	return i;
}

void execute_cmd(char **input_cmd, int background){
	// cout<<"in exec cmd...HI!!!"<<endl;

	// cout<<input_cmd[0]<<endl;
	// cout<<input_cmd[1]<<endl;

	pid_t pid, wpid;
	int status;

	/* execute logic */
	pid=fork();

	// cout<<"pid "<<pid<<endl;

	if(pid < 0){//error in forking
		perror("Error in forking:");
	}

	else if(pid == 0){ //child

		// if(background==0){
		// 	tcsetpgrp(STDIN_FILENO,getpid());
		// 	if(execvp(*input_cmd, input_cmd) ==-1){
		// 		// cout<<"execvp err"<<endl;
		// 		perror("exec failed for child process:");
		// 		//A successful call to execvp does not have a return value .
		// 		//However, a -1 is returned if the call to execvp is unsuccessful.
			
		// 		exit(EXIT_FAILURE); //EXIT_FAILURE: unsuccessful execution of a program
		// 		//the child process exits so that the shell can continue running
		// 	}
		// }
		// else{//child is a background process
		// 	setpgid(getpid(), getpid());
		// cout<<"*input_cmd "<<*input_cmd<<endl;
			if(execvp(*input_cmd, input_cmd) ==-1){
				// cout<<"execvp err"<<endl;
				perror("exec failed for child process:");
				//A successful call to execvp does not have a return value .
				//However, a -1 is returned if the call to execvp is unsuccessful.
			
				exit(EXIT_FAILURE); //EXIT_FAILURE: unsuccessful execution of a program
				//the child process exits so that the shell can continue running
			}
		// }
		
	}

	else{ //parent 
		// cout<<"In parent "<<endl;
		// if(background==0){
		// 	tcsetpgrp(STDIN_FILENO, pid);
		// 	waitpid(pid, &status, WUNTRACED);
		// 	if(!WIFSTOPPED(status)){
		// 		/
		// 	}

		// }
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	// return 1;
	return;
}
