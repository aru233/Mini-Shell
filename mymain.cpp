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
#include <termios.h>
#include <dirent.h>
using namespace std;

#include "open1.h"

#define BUFF_SIZE 1024

typedef struct trie {
	struct trie *child[128];
	int enode;
}mytrie;

void setup_myrc();
void init_env_var();
string getPATH();
string getPrompt();
void disableCanoMode();
// string readInpFromTerm();
string readInpFromTerm(mytrie *root);
mytrie *newNode();
void insertInTrie(mytrie *root, string str);
void getAllFilesInDir(vector<string> &files);
string prefixSearchInTrie(mytrie *root, string str);
void parse_input(char* , char **);
void execute_cmd(char **, int);
int sizeofinp(char **);
int handle_redir(int, int, int ,char **, int);
int findPosOfAmp(char** , char , int);
int findPosOf(char** , string);
int findPosOf1(string , char);
void setInMap(string , string );
string checkInMap(string );
void parseForAlias(char *);
void handle_cd(int , char**);
void handle_export(char*);
const char* checkIfExpo(string);
void handle_echo(string);
void handle_pipes(int , string);
void putInHistFile(string);
void displayHistory();


map<string, string> mp_alias;
map<string, string> mp_env;
char currDir[2048];
int exitStatus;



int main(){
	int background, aliasflag=0;
	string y;
	int pos;
	vector<string> files;

	/* Fetching all the files and dir inside cwd */
	getAllFilesInDir(files);
	/* --- ---*/

	// cout<<"got all dir"<<endl;
	mytrie *root=newNode();
	for(string s : files){
		// cout<<"going to insert "<<s<<" in trie"<<endl;
		insertInTrie(root, s);
	}

	disableCanoMode();

	setup_myrc();

	while(1){
		// getcwd(currDir, 2048);
		// cout<<currDir<<"$";
		string prompt=getPrompt();
		// cout<<mp_env["USER"]<<"@"<<mp_env["HOSTNAME"]<<":"<<currDir<<mp_env["PS1"]<<" ";
		cout<<prompt<<" ";

		fflush(stdout);
		background=0;
		// aliasflag=0;
		char *input_cmd[2048];
		string str;
		str = readInpFromTerm(root);
		// cout<<"Received str in main "<<str<<endl;
		// getline(cin,str);
		if(str.empty()){ // Needed for ctrl+D case...as prog was entering infine loop coz of that
			cout<<endl;
			break;
		}
		mytrie *histRoot=newNode();
		putInHistFile(str);

		char arr[str.length()+1];

		str.copy(arr,str.length());
		arr[str.length()]='\0';

		parse_input(arr, input_cmd);
		
		int len = sizeofinp(input_cmd);
		// cout<<"len "<<len<<endl;

		if(strcmp(input_cmd[0],"exit")==0){
			exit(0);
		}

		// if(len == 1 && strcmp(input_cmd[0], "&")){
		// 	perror("bash: syntax error near unexpected token `&'");
		// 	continue;
		// }

		if(strcmp(input_cmd[0], "history")==0){
			// cout<<"main: in history's if"<<endl;
			displayHistory();
			continue;
		}

		//export a=10
		//checks for export before alias
		if(strcmp(input_cmd[0], "export")==0){
			// cout<<"main: in export's if"<<endl;
			handle_export(input_cmd[1]);
			continue;
		}

		if(strcmp(input_cmd[0], "alias")==0){
			// cout<<"main: in alias if"<<endl;
			string ali;
			ali=str.substr(6);
			// cout<<"ali "<<ali<<endl;
			parseForAlias((char*)ali.c_str());
			continue;
		}

		//alias x="ls"
		//x -l -a
		if((y=checkInMap(string(input_cmd[0]))) != "-1"){
			// cout<<"alias se new str "<<y<<endl;
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
			// cout <<"cmd "<<cmd<<endl;

			strcpy(arr1,cmd.c_str());
			arr1[cmd.length()]='\0';
			parse_input(arr1, input_cmd);
			len=sizeofinp(input_cmd);
		}	

		if(strcmp(input_cmd[0], "open")==0){
			// cout<<"main: in open's if"<<endl;
			handle_open(input_cmd[1]);
			continue;
		}

		if(strcmp(input_cmd[0],"cd")==0){
			// sizeofinp(input_cmd);
			handle_cd(len, input_cmd);
			mytrie *nroot = newNode();
			vector<string> fils;
			getAllFilesInDir(fils);	
			root=nroot;		
			for(string s : fils){
				// cout<<"going to insert "<<s<<" in trie"<<endl;
				insertInTrie(root, s);
			}

			continue; //no need to execute execvp() for cd;	
		}

		/* pipes */
		pos=findPosOf1(str, '|');
		if(pos!=-1){
			// cout<<"FOund pipe .. in main"<<endl;
			handle_pipes(len, str);

			// cout<<"back in main after handle_pipes()"<<endl;
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

		// cout<<"back in main"<<endl;
		

	}//end of outermost while()
}

void displayHistory(){
	char buf[100];
	FILE *mf = fopen("hist.txt","r");
	if(mf!=NULL){
		// cout<<"mf not null "<<endl;
		while(fgets(buf, sizeof(buf), mf)!=NULL){
			cout<<buf<<endl;
		}
	}
	fclose(mf);
}

void putInHistFile(string str){
	FILE* fle=fopen("hist.txt","a+");
	if(fle!=NULL){
		char* line=(char*)str.c_str();
		fprintf(fle, "%s\n",line);
		
	}
	else{
		exitStatus=errno;
		perror("Can't open hist file");
	}
	fclose(fle);
}

// void putInHistTrie(mytrie* histRoot, string str){
// 	insertInTrie(histRoot, str);
// }

void getAllFilesInDir(vector<string> &files){
	// cout<<"getAllFilesInDir()"<<endl;
	struct dirent *dire;
	DIR *d;
	int x=0;
	string dirname;

	getcwd(currDir, 2048);
	// cout<<"currDir "<<currDir<<endl;
	d=opendir(currDir);
	if(d!=NULL){
		// cout<<"in d"<<endl;
		while((dire=readdir(d)) != NULL){
			// cout<<"in d1 "<<dire<<endl;
			// cout<<"dname "<<dire->d_name<<endl;
			// files[x]=dire->d_name;
			dirname=dire->d_name;
			// cout<<"x "<<x<<endl;
			// cout<<" files[x] "<<files[x]<<endl;
			// cout<<" dirname "<<dirname<<endl;
			files.push_back(dirname);
			// cout<<" files[x] "<<files[x]<<endl;
			++x;
		}
		closedir(d);
	}

}

mytrie *newNode(){
	mytrie *newN=new trie();
	int it;
	for(it=0; it<128; it++){
		newN->child[it]=NULL;
	}
	newN->enode=0;
	return newN;
}

void insertInTrie(mytrie *root, string str){
	// cout<<"in insert trie!!, with str "<<str<<endl;
	mytrie *head;
	head=root;
	int childptr, i;
	while(str[i]!='\0'){
		childptr=(int)str[i];
		if(head->child[childptr]==NULL){
			head->child[childptr]=newNode();
			head=head->child[childptr];
		}
		else{
			head=head->child[childptr];
		}
		i++;
	}
	head->enode = 1;
	/* enode =1 marks end of valid string. Say we're inserting "temp" in trie. 
	(en= enode)
	t's en=0 -> e's en=0 -> m's en=0 -> p's en=1
	This 1 at p's enode represents valid string, so that: Say i have a prefix "t".. had this 1 not been set and had I not checked for the same in 
	prefixSearchInTrie(), then my search would've returned "t" as a valid match too (coz looking for shortest match; otherwise "te", "tem", 
	"temp" all considered as valid string, which ain't true) */
}

string prefixSearchInTrie(mytrie *root, string str){

	// cout << "In prefixSearchInTrie(), got string: "<<str<<endl;

	mytrie *head = root;
	int childptr, i;
	i=0;
	while(str[i]!='\0'){//till we parse given str completely
		childptr = (int)str[i];
		// cout<<"str[i]: "<<str[i]<<endl;
		// cout<<"childptr "<<childptr<<endl;
		if(head->child[childptr] == NULL){
			return 0;//unsuccessful search
		}
		head = head->child[childptr];
		i++;
	}
	char ch;
	string autocomp;
	int noOfChild, imp;
	autocomp=str;//we'll start looking for autocompletion suggestions for given prefix 'str'
	while(head->enode != 1){//looking for shortest match out of all available matches. As enode=1 means that upto this point we 
		//have something(we must have put enode =1 when we inserted sth in trie. 1 indicates nothing more ahead of that node...
		//now if during some next traversal, some new string inserted, we navigate further into that enode=1 node..)
		noOfChild = 0;
		for(imp=0;imp<128;imp++){ 
			if(head->child[imp] != NULL){
				noOfChild++;
				if(noOfChild >= 2){
					return "";//if more than 2 possibilities, we dont display either on pressing tab
				}
				ch=imp;
				autocomp+=ch;
			}
		}
		head=head->child[(int)ch];
	}
	// cout<<"autocomp "<<autocomp<<endl;
	// return autocomp;

	if(head->enode == 1)
		return autocomp;
	else
		return "";
}



string getPrompt(){
	getcwd(currDir, 2048);
	string prom=mp_env["USER"]+"@"+mp_env["HOSTNAME"]+":"+(string)currDir+mp_env["PS1"];
	return prom;
}

void disableCanoMode(){
	struct termios terms;
	tcgetattr(0, &terms);
	terms.c_lflag &= ~(ICANON);
	tcsetattr(0, TCSAFLUSH, &terms);

	
}
string readInpFromTerm(mytrie *root){
	char ch=0;
	string str;
	string str1;
	int i=0;
	int len, len1;
	int start, m=0;
	char *buf[20];
	string pref;
	string searchRes;
	string finalRes="";
	while((ch=getchar()) !='\n'){
		// cout<<"char ch "<<ch<<endl;
		if(ch==9){//Tab

			// cout << "TAB aaya"<<endl;
			str1=str.substr(0,i);//send the str to search for autocompl suggestion
			// cout<<"Tab mai string: "<<str<<endl;
			int iu;
			iu=1;
			char _str[1024];
			char cp[1024];
			strcpy(_str, str1.c_str());
			buf[0] = strtok(_str," ");
			// cout<<"buf[0] "<<buf[0]<<endl;
			while((buf[iu] = strtok(NULL," "))!= NULL){
				// cout<<"buf[whatev] "<<buf[iu]<<endl;
				iu++;
			}
			buf[iu] = NULL;
			//cout<<clargs[n-1]<<endl;
			pref = buf[iu-1];
			// cout<<"Pref: "<<pref<<endl;
			searchRes = prefixSearchInTrie(root,pref);
			if(searchRes == ""){//entered prefix is wrong
				cout<<"Invalid name entered"<<endl;
				continue;
			}
			// cout<<"SearchRes: "<<searchRes<<endl;
			len1=searchRes.length();
			strcpy(cp,searchRes.c_str());
			len=strlen(buf[iu-1]);
			start=iu-len;
			// cout<<"str oop: "<<str<<endl;
			int pp=0;
			finalRes=buf[pp];
			for(pp=1;pp<iu-1;pp++){
				string m=buf[pp];
				finalRes=finalRes+" "+m;
			}
			finalRes=finalRes+" "+searchRes;
			string prom=getPrompt();
			// cout<<"strrr "<<finalRes<<endl;
			// cout<<finalRes;
			printf("\r                                                                                                              ");
			printf("\r%s",(char*)(prom+" "+finalRes).c_str());
			// cout<<"Final Res "<<finalRes<<"_";
			str=finalRes;
		}
		else if(ch==127){//Backspace
			string prom=getPrompt();
			printf("\r                                                                                                              ");
			if(i>0){
				str=str.substr(0,i-1);
				i--;
			}			
			printf("\r%s",(char*)(prom+" "+str).c_str());
		}

		// else if(ch==18){//CTRL+R

		// }
		

		else{
			// cout<<"Read i/p char "<<ch;
			str+=ch;
			// cout<<" string so far "<<str;
			i++;
		}
	}
	// cout<<" Received input str "<<str;
	// cout<<"My str "<<str;
	return str;
}

//export a=10
void handle_export(char *expo){
	
	char *var[5];
	var[0]=strtok(expo,"=");
	var[1]=strtok(NULL,"=");
	FILE* fle=fopen("myrc.txt","a+");
	// cout<<"var[0] "<<var[0]<<endl;
	// cout<<"var[1] "<<var[1]<<endl;
	if(fle!=NULL){
		fprintf(fle, "%s=%s\n", var[0], var[1]);
		fclose(fle);
	}
	else{
		exitStatus=errno;
		perror("Can't open file");
	}
}

const char* checkIfExpo(string str){
	char buf[50];
	FILE *mf = fopen("myrc.txt","r");
	if(mf!=NULL){
		// cout<<"mf not null "<<endl;
		while(fgets(buf, sizeof(buf), mf)!=NULL){
			char *path[1024];
			path[0] = strtok(buf, "=");
			if(strcmp(path[0], str.c_str()) == 0){
				path[1] = strtok(NULL,"=\n");
				string s=path[1];
				return s.c_str();
			}	
		}
	}
	
	fclose(mf);
	return "-1";
}

void setup_myrc(){

	init_env_var();
	// char buf[50];
	// FILE *mf = fopen("myrc.txt","a+");
	// if(mf!=NULL){
	// 	// cout<<"mf not null "<<endl;
	// 	while(fgets(buf, sizeof(buf), mf)!=NULL){
	// 		char *path[1024];
	// 		path[0] = strtok(buf, "=");
	// 		if(strcmp(path[0], str.c_str()) == 0){
	// 			path[1] = strtok(NULL,"=\n");
	// 			string s=path[1];
	// 			return s.c_str();
	// 		}	
	// 	}
	// }

	FILE* fle=fopen("myrc.txt","a+");
	if(fle!=NULL){
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
		fprintf(fle, "jpeg=%s\n", "/usr/bin/eog");
		fprintf(fle, "jpg=%s\n", "/usr/bin/eog");
		fprintf(fle, "png=%s\n", "/usr/bin/eog");

		fclose(fle);
	}
	else{
		exitStatus=errno;
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
	char *ar[10];

	ar[0] = strtok(stre,"=");
	ar[1] = strtok(NULL,"=");

	// cout<<"key "<<ar[0]<<endl;
	// cout<<"val"<<ar[1]<<endl;

	mval=string(ar[1]);
	mval.erase(remove(mval.begin(), mval.end(), '\"' ),mval.end());
	mval.erase(remove(mval.begin(), mval.end(), '\'' ),mval.end());
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
	// cout<<"Here in count_pipes() "<<endl;
	*pipcomcount=1;
	pipcmd[0]=strtok((char*)str.c_str(), "|");

	// cout << "pipcmd[0]: "<<pipcmd[0]<<endl;
	while((pipcmd[*pipcomcount] = strtok(NULL, "|")) != NULL){
		// cout<<"pipcomcount "<<*pipcomcount<<", pipcmd[*pipcomcount] "<<pipcmd[*pipcomcount]<<endl;
		(*pipcomcount)++;
		// cout<<"pipcomcount "<<*pipcomcount<<endl;
		// cout<<"pipcomcount "<<*pipcomcount<<", pipcmd[*pipcomcount] "<<pipcmd[*pipcomcount]<<endl;
	}
	pipcmd[*pipcomcount]=NULL;
}

void handle_pipes(int leng, string str){
	int fd[2];
	int fdpos;
	int pt, status;
	pid_t pid;
	char* pipcmd[1024];
	int pipcomcount=1, redir_flag=0;

	pipcmd[0]=strtok((char*)str.c_str(), "|");
	while((pipcmd[pipcomcount] = strtok(NULL, "|")) != NULL){
		pipcomcount++;
	}
	pipcmd[pipcomcount]=NULL;
	pt=-1;
	fdpos=0;

	while(pt++<pipcomcount){
		pipe(fd);
		pid=fork();
		
		if(pid==-1){
			exitStatus=errno;
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
			// cout<<"arr[0] "<< ark[0]<<endl;
			// cout<<"arr[1] "<< ark[1]<<endl;

			if(pt<pipcomcount-1){//for last command, o/p to be written to terminal only
				dup2(fd[1], STDOUT_FILENO);
			}
			close(fd[0]);
			// cout<<"in handle_pipes() not redir case: "<<arr[0]<<endl;if(pt==pipcomcount-1){//checking redirection in last cmd
			
			if(pt==pipcomcount-1){//checking redirection in last cmd
				/* Redirection */
				int pos, background=0;
				int len = sizeofinp(ark);;
				pos=findPosOf(ark, ">");
				// cout<<"pos of > in.. "<<pos<<endl;
				// cout<<"pos1 "<<pos<<endl;
				if(pos!=-1){//a redirection case
					handle_redir(len, pos, 1, ark, background);//1: ">"
					// continue; // No need to execute execute_cmd() as it's already handled by handle_redir()
					// cout<< "came back after handling redir"<<endl;
					redir_flag=1;
				}

				pos=findPosOf(ark, ">>");
				if(pos!=-1){//a redirection case
					handle_redir(len, pos, 2, ark, background);//2: ">>"
					redir_flag=1;
				}
				/* ...Redirection End...*/
			}
			else if(pt!=pipcomcount-1 || redir_flag==0){
				if(execvp(ark[0], ark) ==-1){
					perror("exec failed for child process:");
					//A successful call to execvp does not have a return value .
					//However, a -1 is returned if the call to execvp is unsuccessful.
					exit(127); //EXIT_FAILURE: unsuccessful execution of a program
					//the child process exits so that the shell can continue running
			
				}	
			}						
			
		}
		else{//parent
			// cout<<"in handle_pipe parent"<<endl;
			// pt++;
			while((pid=waitpid(-1, &status, WCONTINUED|WNOHANG|WUNTRACED)) > 0){
				if (WIFEXITED(status)) {
		            exitStatus=1;//done
		        } 
		        else if (WIFSTOPPED(status)) {
		            exitStatus=1;//suspended
		        } 
		        else if (WIFCONTINUED(status)) {
		            exitStatus=3;//continue
		        }
			}
			// waitpid(pid, &status, 0);
			// if(WIFEXITED(status)){
			// 	exitStatus=WEXITSTATUS(status);
			// }
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

	// cout<<"str in echo "<<str<<endl;
	int ech_l1=str.length(), ech_l2;
	string str1;
	str.erase(remove( str.begin(), str.end(), '\"' ),str.end());
	if(str[5]=='\''){
		str.erase(remove( str.begin(), str.end(), '\'' ),str.end());
	}
	// cout<<"str in echo ..after "<<str<<endl;

	ech_l2=str.length();
	if(ech_l1-ech_l2==1){//Quote mismatch
		exitStatus=-1;
		cout<<"QUOTE mismatch"<<endl;
		return;
	}

	int i=5;
	const char* xy;
	while(str[i]!='$' && i<ech_l2){
		i++;
	}
	if(str[i]=='$'){
		str1=str.substr(i+1);
		if(str1=="$"){
			cout<<getpid()<<endl;
		}
		else if(str1=="?"){
			cout<<exitStatus<<endl;
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
		else if(strcmp((xy=checkIfExpo(str1)), "-1") != 0){
			cout<<xy<<endl;
			exitStatus=0;
			return;
		}
		else{
			cout<<str.substr(5)<<endl;
			exitStatus=0;
			return;
		}
	}
	else{
		cout<<str.substr(5)<<endl;
		exitStatus=0;
		return;
	}
}

void handle_cd(int len, char** input_cmd){
	// cout<<"here in cd"<<endl;
	sizeofinp(input_cmd);
	string filename;
	string ss=mp_env["HOME"];
	char home[1024]; 
	strcpy(home, ss.c_str());

	// cout<<"ss"<<ss<<endl;
	// char home[1024]="/home/arushi";
	int x;


	if(len==1 || string(input_cmd[1]).empty() ){// writing just cdcd takes us to home dir
		sprintf(currDir, "%s", home);

		// getcwd(currDir, 2048);
		// cout<<"currDir "<<currDir<<endl;
		if(chdir(currDir)!=0){
			exitStatus=errno;
			perror("Error in changin dir");
		}
		return;
	}

	filename = input_cmd[1];
	// cout<<"cd: filename "<<filename<<endl;
	
	// cout<<"len in cd "<<endl;
	
	
	// str path;
	if(filename[0] == '~'){ // cd ~/IIITH types should also work
		sprintf(currDir, "%s%s", home, input_cmd[1]+1);

		// getcwd(currDir, 2048);
		// cout<<"currDir "<<currDir<<endl;
		if(chdir(currDir)!=0){
			exitStatus=errno;
			perror("Error in changin dir");
		}

	}
	
	else if((x=chdir(filename.c_str()))!=0){
		// cout<<"x "<<x<<endl;
		exitStatus=errno;
		perror("Error in changin dir");
	}
	// continue;
}

int handle_redir(int len, int pos, int flag, char** input_cmd, int background){
	if(string(input_cmd[pos+1]).empty()){
		errno=ENOENT;
		exitStatus=errno;
		perror("Enter a valid filename:");
		return 0;
	}
	pid_t pid, wpid;
	int status, fd1;
	pid=fork();

	if(pid < 0){//error in forking
		exitStatus=errno;
		perror("Error in forking:");
	}
	else if(pid == 0){ //child
		if(flag==1){
			// cout<< "Here for >"<<endl;
			if((fd1=open(input_cmd[pos+1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR|S_IWUSR)) == -1){
				exitStatus=errno;
				perror("Error in opening file");
			}
		}
		if(flag==2){
			// cout<< "Here for >2 "<<endl;
			if((fd1=open(input_cmd[pos+1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR|S_IWUSR)) == -1){
				exitStatus=errno;
				perror("Error in opening file");
			}
		}
		
		dup2(fd1, STDOUT_FILENO);
		input_cmd[pos]=NULL;

		if(execvp(input_cmd[0], input_cmd) ==-1){
			// cout<<"execvp err"<<endl;
			perror("exec failed for child process:");
		//A successful call to execvp does not have a return value .
		//However, a -1 is returned if the call to execvp is unsuccessful.			
			exit(127); //EXIT_FAILURE: unsuccessful execution of a program
		//the child process exits so that the shell can continue running
		}
	}

	else{ //parent 
		// cout<<"In parent "<<endl;
		waitpid(pid, &status, 0);
		if(WIFEXITED(status)){
			exitStatus=WEXITSTATUS(status);
		}while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
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
		// cout<<"i "<<i<<"  "<<input_cmd[i]<<endl;
		i++;
	}
	return i;
}

void execute_cmd(char **input_cmd, int background){
	pid_t pid, wpid;
	int status;

	/* execute logic */
	pid=fork();

	// cout<<"pid "<<pid<<endl;

	if(pid < 0){//error in forking
		exitStatus=errno;
		perror("Error in forking:");
	}

	else if(pid == 0){ //child
		if(execvp(*input_cmd, input_cmd) ==-1){
			// cout<<"execvp err"<<endl;
			perror("exec failed for child process:");
			//A successful call to execvp does not have a return value .
			//However, a -1 is returned if the call to execvp is unsuccessful.
			exit(127); //EXIT_FAILURE: unsuccessful execution of a program
			//the child process exits so that the shell can continue running
		}
		
	}

	else{ //parent 
		waitpid(pid, &status, 0);
		if(WIFEXITED(status)){
			exitStatus=WEXITSTATUS(status);
		}
		// do {
		// 	wpid = waitpid(pid, &status, WUNTRACED);
		// } while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	// return 1;
	return;
}
