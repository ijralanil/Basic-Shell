#include"header.h"

//general functions
void make_array(char **argv);
void help();
void get_prompt(char *str);			//create prompt data
void input_scan(char *input,char **argv);	//scan input command for ;  > |


//";" and in single shot mode related functions
void internal_command(char*);
void external_command(char **,int);
void commands(char **argv,int i);
void seperator(char *comm,char **argv,int select);
void comm_seperate(char *input,char **argv);

//indirection related functions
void extraction(char *input,char **argv);
void extract_it(char **argv,char *ch,int z);
//pipe related functions
void pipe_resolve(char *input,char **argv);
void pipes(char **argv,int in_read,int pipe1,int out_write,int pipe2);
//void pipes(char **argv,int in_read,int out_write);


//history related functions
typedef struct st
{
	char cmd[30];
	struct st *next;
}HIST;

HIST *head,*tail;

void make_history(char**,char*);
void history(char *);
void hist_print(HIST *,int);


int hist_count;

void input(char *str)
{
	scanf(" %[^\n]",str);	//take input
}

void main(int argc,char** argv)
{

	char prompt[40],input_cmd[100],prompt1[30];
	char *arguments[5];	//argv holds -c option, but argument will hold commands and their parts


	
	if(argc>1){	

		if(!strcmp(argv[1],"--help")){		//if asked for help
			help();return;
		}	

		if(!strcmp(argv[1],"-c")){		//single-command mode 
			commands(argv,2);
			make_history(argv,input_cmd); 
			history(input_cmd);
			exit(0);	//exit the program
		}

	}

	else{					//continuous input mode

			get_prompt(prompt1);	//get prompt name

		while(1)
			{
			sprintf(prompt,"<%d %s>",hist_count,prompt1);
prompt:			printf("%s",prompt);	
		
			scanf(" %[^\n]s",input_cmd);	//take input
			history(input_cmd);			
			input_scan(input_cmd,arguments);		
			bzero(input_cmd,100);	
		}

	}



}


//*****
// In continuous mode, scan for input commands.
//This function scans for ; < > | in command and executes them.
//input-> command string

//void input_scan(char *input,char **argv)	//scan input command for ; < > |
void input_scan(char *input,char **argv)	//scan input command for ; < > |
{


	if(strchr(input,'|')){
		pipe_resolve(input,argv);
	}
	else if(strchr(input,';')){

		comm_seperate(input,argv);	
	}
	else if(strchr(input,'>')){
		extraction(input,argv);
	}
	else{
		comm_seperate(input,argv);	
	}

}



//it is called when input command present a '|' as reguler expression.
//This function is called only in continuous mode. it is a function which continuously creates pipes and childs to perform multiple pipe implementation.
//input- is where to read for command
//output is where to write for command
//abc where actual output is read.

void pipe_resolve(char *input,char **argv)
{
	
	char *ptr,*str,ch[100];
	int p[2]={100,0},q[2]={100,0},i;
	int count=1,inputs,output,abc,input1,output1;
	
	/////////////////////count no.of pipes//////////	
	str=input;
	while( ( str=strchr(str,'|') ) ){
		str++;
		count++;
	}
	
	///////////////////////////////////////////////
	str=strtok_r(input,"|",&ptr);

	while(count>-1){

		

		if(count){
		make_array(argv);
		seperator(str,argv,0);	//fill the argv
	

		if( count%2==0 ){
			pipe(p);
			inputs=q[0];output=p[1];abc=p[0];
			input1=q[1];output1=p[0];
		}
		else {
			pipe(q);
			inputs=p[0];output=q[1];	abc=q[0];
			input1=p[1];output1=q[0];
		}



		pipes(argv,inputs,input1,output,output1);
		}

		if(count==0)
		{
			

			while( (i=read(abc,ch,100) ) ){
				ch[i]='\0';
			
				printf("%s",ch);
				bzero(ch,100);
			}
				
	
			close(abc);	
			close(p[0]);	
		}
		count--;
	str=strtok_r(NULL,"|",&ptr);
	

	}
	__fpurge(stdin);

}

//This function runs external commands and puts their output in pipe->(out_write).
//argv-> is having commands to execute
//in_read-> contains pipe descriptor from where data is read by execvp.(when first time called is of no use.)
//out_write-> pipe descriptor to write command output by exec
//void pipes(char **argv,int in_read,int out_write)
void pipes(char **argv,int in_read,int pipe1,int out_write,int pipe2)
{
	//printf("PIPES now\n");
	if(fork()==0){
		dup2(in_read,0);	//close & copy read end of pipe to input stream descriptor
		dup2(out_write,1);	//close & copy write end of pipe to output stream descriptor
		dup2(out_write,2);	//close & copy write end of pipe to error stream descriptor
		close(pipe1);	
		close(pipe2);	
	////////////////////////////
	
			if( execvp(argv[0],argv) <0 ){	
				perror("");
				close(out_write);
				close(in_read);
				exit(1);
			}	
			exit(0);	
		//handle error here
	}//orphan child
	else{
		wait(0);
		close(out_write);
		close(in_read);

	}
}




//it is called when input command present a '>' as reguler expression.

void extraction(char *input,char **argv)
{
	char *ptr,ch[20];
	int i,error_flag=0;
	char *p=strtok_r(input,">",&ptr);

//////////////////// which stream to copy 1>abc.c or 2>abc.c/////////////
// 
	i=strlen(p);
	if ( p[i-2]==' ' && p[i-1]=='2' ){
		error_flag=1;
		//printf("error %d\n",error_flag);
		p[i-1]='\0';
	}
	else if( p[i-2]==' ' && p[i-1]=='1'){
		//printf("%c\n",p[i-1]);
		p[i-1]='\0';
	
	}
	else if( p[i-2]==' ' && p[i-1]=='0'){
		//printf("%c\n",p[i-1]);
		p[i-1]='\0';
	
	}
	
	

////////////////////
	while(p){

		make_array(argv);
		seperator(p,argv,0);
		p=strtok_r(NULL,">",&ptr);
		strcpy(ch,p);
		break;
	}	
		extract_it(argv,ch,error_flag);	

}








//it is called when input command present a ';' as reguler expression.
//void comm_seperate(char *input,char **argv,int no)  //no. to differenciate b/w the -c and continuous mode
void comm_seperate(char *input,char **argv)
{
	char *ptr;
	char *p=strtok_r(input,";",&ptr);
	while(p){

		make_array(argv);
		seperator(p,argv,1);
		p=strtok_r(NULL,";",&ptr);
	}	

	
}

//it sepeartly puts the command and its options. and fills the argv
//comm->command
//argv-> where command needs to be put
//select->decides whether command should be executed or not.
void seperator(char *comm,char **argv,int select)
{
	int i=0;
	char *p,*ptr;
	p=strtok_r(comm," ",&ptr);
	while(p){
		strcpy(argv[i],p);
		i++;
		p=strtok_r(NULL," ",&ptr);
	}
	
			argv[i]='\0';
		

	if(select)
		commands(argv,0);
}



//****
//This function generates string for prompt 
//str-> is string where prompt should be stored.

void get_prompt(char *str)			//create prompt data
{
	char login_name[10],host_name[20];

		getlogin_r(login_name,10);	//get user name
		gethostname(host_name,20);	//get host name
		sprintf(str,"%s %s",login_name,host_name);
		printf("str- %s\n",str);
		
}






//****
//print the help menu, contains internal commands


void help()
{
	printf("./shell options commands\n");
		printf("Options \n\t-c: run the command and terminate, if not, wait for input continuously.\n");
		printf("commands \n");
		printf("\tHist: last 10 commands used\n\tCpid: pid of shell\n\tPpid: parent pid of shell\n\tQuit: terminate shell\n\tcd Dir: change directory\n");

}


//****
//determine the command type and execute them
//i determines location where command resides

void commands(char **argv,int i)
{
		
		if(!strcmp(argv[i],"Hist"))
			internal_command("Hist");		
		else if(!strcmp(argv[i],"Cpid"))		
			internal_command("Cpid");		
		else if(!strcmp(argv[i],"Ppid"))		
			internal_command("Ppid");		
		else if(!strcmp(argv[i],"Quit"))		
			internal_command("Quit");	
		else{
			if(i)		//if this function called with -c option, go with 1.
			external_command(argv,1);
			else		//else go with 0.
			external_command(argv,0);
			
		}
}

//****
//this function runs Commands internal to our Shell(program).
//cmd is string contain internal command.

void internal_command(char* cmd)
{
		if(!strcmp(cmd,"Hist")){
			hist_print(head,1);
		}
		else if(!strcmp(cmd,"Cpid")){
			printf("Shell PID=%d\n",getpid());
		}		
		else if(!strcmp(cmd,"Ppid")){		
			printf("Shell PPID=%d\n",getppid());
		}		
		else if(!strcmp(cmd,"Quit")){		
			exit(0);
		}		
		
}

//****
//this function runs the bash commands 
//argv-> cmd line arguments given to program
//z-> if single-command mode only, z=1, call make history, 
void external_command(char **argv,int z)
{
	char ch[100];
	if(z==1){
		make_history(argv,ch);	//this function converts all the array members into single array.
		//put data in history
		argv=argv+2;	//modify the argv and make argv[0]=command name
	}

	if(fork()==0){
		if( execvp(argv[0],argv) <0 ){	
			perror("");
		}
		exit(0);
	}
	else{
		wait(0);
	
	}
}

//****
//this functions converts the cmd-line argument array into the single array to store it into history.

//argv-> cmd line string arguments,
// ch-> array in which result to be stored,
// i is where to start 1-> continuous mode, 2 for single-command mode
void make_history(char** argv,char* ch)	
{
	int i,j=0;
	for(i=2;argv[i];i++){
		strcpy(ch+j,argv[i]);	
		j=j+strlen(argv[i]);
		ch[j++]=' ';
	}
		ch[j]='\0';
}



void make_array(char **argv)
{
	int i=0;
	char static *arr[5],j=0;
	if(j==0){			//only once create dynamic memory

	for(i=0;i<5;i++)	
		arr[i]=calloc(10,1);
	j=1;
	}
	for(i=0;i<5;i++)
		argv[i]=arr[i];
}



//****
//this function runs the bash commands for ">"  
//argv-> cmd line arguments given to program
//path->for filename to open


//works for both internal as well as external commands

//z-> if single-command mode only, z=1, call make history, 
void extract_it(char **argv,char *path,int z)
{
	char ch[100];
	int p[2],i=0;

	 if(!strcmp(argv[0],"Quit"))		
		internal_command("Quit");		

//*****************pipe*************
	pipe(p);
	if(fork()==0){
		if(z)
		dup2(p[1],2);
		else{
		close(p[0]);	//close pipe read end
		close(1);
		dup(p[1]);
		}
	////////////////////////////

	
		if(!strcmp(argv[0],"Hist"))
			internal_command("Hist");		
		else if(!strcmp(argv[0],"Cpid"))		
			internal_command("Cpid");		
		else if(!strcmp(argv[0],"Ppid"))		
			internal_command("Ppid");		
		else{
		
			if( execvp(argv[0],argv) <0 ){	
				perror("");
				close(p[1]);
				exit(1);
			}
	
		}
			close(p[1]);
			exit(0);	
		
	}
	else{
		int x;
		close(p[1]);	//close pipe write end
		wait(&x);
		
		if(WEXITSTATUS(x)==0){	
			x=open(path,O_CREAT|O_WRONLY|O_TRUNC,0644);
			if(x<0){
				perror("open");
				return;
			}
		}

		while( (i=read(p[0],ch,100) ) ){
			ch[i]='\0';
			
			write(x,ch,strlen(ch)+1);
			bzero(ch,100);
		}
	

	}
}



void history(char *cmd)
{
	HIST *temp=malloc(sizeof(HIST));
	strcpy(temp->cmd,cmd);
	if(!hist_count){
		head=temp;
		tail=temp;
		temp->next=0;	
		hist_count++;
	}		
	else{
		tail->next=temp;	
		temp->next=0;
		tail=temp;
		hist_count++;
	}

	if(hist_count>10){
		temp=head;
		head=head->next;
		free(temp);	
	}


}

void hist_print(HIST *temp,int no)
{
	
	if(temp){
		printf("%d  %s\n",no,temp->cmd);
		hist_print(temp->next,no+1);

	}

}
