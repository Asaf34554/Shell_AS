#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h" // Standard I/O functions
#include "errno.h"
#include "stdlib.h"
#include "unistd.h" // Symbolic Constants and Types, Function Declarations
#include <string.h>
#include "shell.h"
#include <signal.h>

#define MAX_COMMAND_SIZE 1024

void redct(char** argv,char *outfile,int* i,int* redirect){
    if (!strcmp(argv[*i - 2], ">") || !strcmp(argv[*i - 2], "2>") || !strcmp(argv[*i - 2], ">>")) {
        if (!strcmp(argv[*i - 2], ">"))
            *redirect = 1;

        else if(! strcmp(argv[*i - 2], "2>"))
            *redirect = 2;

        else // ">>"
            *redirect = 3;

        argv[*i - 2] = NULL;
        strcpy(outfile,argv[*i-1]);
        *i = *i - 2;
    }
    else
        *redirect = 0;

}

void if_then_else(char *comm,char *outcmd,int *t_flag,int *e_flag,const int len){
    /* Function to handle if-then-else-fi conditional statements */

    // Extract data
    char cmd[MAX_COMMAND_SIZE] = "\0";
    char tmp[MAX_COMMAND_SIZE] = "\0";
    strcpy(tmp,comm);
    int lens = 0;
    char *argv[10];
    divide_cmd(argv,&lens,tmp," ");
    char then_cmd[MAX_COMMAND_SIZE] = "\0",do_cmd[MAX_COMMAND_SIZE] = "\0",else_cmd[MAX_COMMAND_SIZE] = "\0";
    char else_do_cmd[MAX_COMMAND_SIZE] = "\0",fi_cmd[MAX_COMMAND_SIZE] = "\0";

    // Build the command to run in system and check if it's valid
    for (int i = 1; i < lens; ++i)
    {
        strcat(cmd, argv[i]);
        strcat(cmd, " ");
    }
    fgets(then_cmd, MAX_COMMAND_SIZE, stdin); // Read 'then' command
    fgets(do_cmd, MAX_COMMAND_SIZE, stdin); // Read 'do' command

    fgets(else_cmd, MAX_COMMAND_SIZE, stdin); // Read 'else' command

    if(!strcmp(else_cmd,"else\n")){
        fgets(else_do_cmd, MAX_COMMAND_SIZE, stdin); // Read 'else-do' command
        fgets(fi_cmd, MAX_COMMAND_SIZE, stdin); // Read 'fi' command
    }
    else if(!strcmp(else_cmd,"fi\n")){
        strcpy(fi_cmd,else_cmd); // Copy 'fi' command
    }
    
    if (!strcmp(then_cmd, "then\n") && !strcmp(fi_cmd, "fi\n"))
    {
        if (!system(cmd))
        {
            // Execute 'do' command
            *t_flag = 1;
            strcpy(outcmd,do_cmd);
        }
        else
        {
            // Execute 'else-do' command
            *e_flag = 1;
            strcpy(outcmd,else_do_cmd);
        }
    }
    else
    {
        printf("Bad syntax in if-then-else-fi, enter again\n");
    }
}

void divide_cmd(char **cmdbuffer, int *n, char *cmd, const char* ch)
{
    /* Function to divide command into tokens */

    char *tkn;
    tkn = strtok(cmd, ch);
    int ndex = -1;
    while (tkn)
    {
        cmdbuffer[++ndex] = malloc(sizeof(tkn) + 1);
        strcpy(cmdbuffer[ndex], tkn);
        unspace(cmdbuffer[ndex]); // Remove leading and trailing spaces
        tkn = strtok(NULL, ch);
    }
    cmdbuffer[++ndex] = NULL;
    (*n) = ndex;
}

void unspace(char* cmd){
    /* Function to remove leading and trailing spaces from a string */

    if(cmd[0] == ' ' || cmd[0] == '\n'){
        memmove(cmd,cmd+1,strlen(cmd));
    }
    if(cmd[strlen(cmd)-1] == ' ' || cmd[strlen(cmd)-1] == '\n'){
        cmd[strlen(cmd)-1] = '\0';
    }
}

void pipe_hndl(char **cmdbuf, int index){
    /* Function to handle pipe commands */
    char outfile[1024] = "\0";
    if (index > 50)
    {
        return;
    }

    int fd[50][2]; // 50 pipes
    int indx =0,red=0;
    char *cmd[50];
    

    for (int i = 0; i < index; i++)
    {
        
        divide_cmd(cmd, &indx, cmdbuf[i], " ");
        redct(cmd,outfile,&indx,&red);
        printf("%s\n",cmd[i]);
        if (i != index - 1){
            if (pipe(fd[i]) == -1){
                perror("ERROR IN PIPE!\n");
                return;
            }
        }

        /* Child process */
        if (fork() == 0)
        {
            signal(SIGINT,SIG_DFL);
            if (i != index - 1)
            {
                dup2(fd[i][1], STDOUT_FILENO);
                close(fd[i][0]);
                close(fd[i][1]);
            }
            
            if(i == index-1 && red != 0){
                int tfd;
                if (red == 1) { // ">"
                    tfd = creat(outfile, 0660); 
                    dup2(tfd, STDOUT_FILENO);
                    close(tfd);
                    /* stdout is now redirected */
                }
                else if(red == 2){ // "2>"
                    tfd = creat(outfile, 0660); 
                    dup2(tfd, STDERR_FILENO); 
                    close(tfd); 
                    /* stderr is now redirected */
                }
                else if (red == 3){ // ">>"
                    tfd = open(outfile, O_WRONLY | O_APPEND | O_CREAT, 0660);
                    dup2(tfd, STDOUT_FILENO); 
                    close(tfd); 
                    /* stdout is now redirected */
                }
            }
            if(i != 0)
            {
                dup2(fd[i - 1][0], STDIN_FILENO);
                close(fd[i - 1][1]);
                close(fd[i - 1][0]);
            }
            
            

            /* redirection of IO ? */

            execvp(cmd[0], cmd);
            exit(1);
        }
        /* Parent process */
        if (i != 0)
        {
            close(fd[i - 1][0]);
            close(fd[i - 1][1]);
        }
        wait(NULL);
        // Free memory allocated for cmd array
        for (int j = 0; j <= indx; j++) {
            free(cmd[j]);
        }
    }
}



void read_var(char** argv,vari *varbuff,int *varcount,const int len){
    /* Function to read variable values */

    int idx=0;
    char rcmd[MAX_COMMAND_SIZE]="\0";
    char* rargv[10];
    fgets(rcmd,MAX_COMMAND_SIZE,stdin);
    divide_cmd(rargv,&idx,rcmd," ");
    for(int i=1;i<len;i++){
        sprintf(varbuff[*varcount].vname,"$%s",argv[i]);
        if(i-1 < idx){
            strcpy(varbuff[*varcount].vvalue,rargv[i-1]);
        }
        else
        {
            strcpy(varbuff[*varcount].vvalue," ");
        }
        (*varcount)++;
    } 
}

char* get_vval(char* vname,vari* vars, const int numv){
    /* Function to get the value of a variable */

    for(int i=0;i<numv;i++){
        if(!strcmp(vars[i].vname,vname)){
            return vars[i].vvalue;
        }
    }
    return NULL;
}

void add_var(char *cmd,vari *vbuff,int *varcount){
    /* Function to add a variable */

    int index = 0;
    char *token = strtok(cmd," ");
    while (token != NULL)
    {
        if(index == 0){
            strcpy(vbuff[*varcount].vname,token);
                  
        }
        if(index == 2)
        {
            strcpy(vbuff[*varcount].vvalue,token); 
        }
        token = strtok(NULL," ");
        ++index;

    }
    (*varcount)++;
}

void cd_dir(char* argv [],int len){
    /* Function to handle cd command */

    if(len == 1 || !strcmp(argv[1],"~")){
        const char *home_dir = getenv("HOME");
        if(home_dir == NULL)
            perror("Error: HOME environment variable not set.\n");

        if(chdir(home_dir) != 0){
            perror("cd");
        }
    }
    else if(len == 2){
        if(chdir(argv[1]) != 0){
            perror("cd");
        }
    }
    else{
        char dir[MAX_COMMAND_SIZE] = "";
        for(int i = 1;i<len;i++){
            strcat(dir,argv[i]);
            if(i < len -1)
                strcat(dir," ");
        }
        if((dir[0] == '\'' && dir[strlen(dir) - 1] == '\'') ||
            (dir[0] == '\"' && dir[strlen(dir) - 1] == '\"')){
                dir[strlen(dir) - 1] = '\0';
                if(chdir(dir+1) != 0){
                    perror("cd");
                }
            }
        else{
            printf("too many arguments in cd, try using quotes signs on the folder name\n");
        }
    }

}

void sig_int(int sig){
    /* Signal handler for SIGINT (CTRL+C) */

    printf("\nYou typed Control-C!\n");
}

int main() {
    /* Main function */

    char then_else_cmd[MAX_COMMAND_SIZE];
    char command[MAX_COMMAND_SIZE];
    char last_cmd[MAX_COMMAND_SIZE];
    char pipe_cmd[MAX_COMMAND_SIZE];
    char *token;
    char outfile[MAX_COMMAND_SIZE];
    int i, fd, amper, redirect, retid, status,pipecount=0,varcount=0,t_flag = 0,e_flag=0;
    char *argv[10];
    char *cmdbuffer[256];
    vari varbuff[20];
    char prompt[256];
    strcpy(prompt,"hello");

    // Handles CTRL+C Signal and ignores it
    signal(SIGINT,sig_int);

    while (1)
    {   
        if(t_flag){
            strcpy(command,then_else_cmd);
            command[strlen(command) - 1] = '\0'; /* code */
            t_flag = 0;
        }
        else if(e_flag){
            strcpy(command,then_else_cmd);
            command[strlen(command) - 1] = '\0'; /* code */
            e_flag = 0;
        }
        else
        {
            printf("%s: ",prompt);    
            fgets(command, MAX_COMMAND_SIZE, stdin);
            command[strlen(command) - 1] = '\0'; /* code */
        }
 
        if(strncmp(command,"!!",2) ){
            strcpy(last_cmd,command);
        }
        
        if(! strcmp(command,"quit")){
            exit(0);
        }

        if (!strncmp(command,"$",1))
        {
            add_var(command,varbuff,&varcount);
            continue;
        }
        

        /* parse command line */ 
        strcpy(pipe_cmd,command);
        i = 0;
        token = strtok (command," ");
        while (token != NULL)
        {
            argv[i] = token;
            token = strtok (NULL, " ");
            i++;
            if (token && !strcmp(token, "|")){ 
                
                divide_cmd(cmdbuffer, &pipecount, pipe_cmd, "|");
                pipe_hndl(cmdbuffer, pipecount);
                break;
            }
        }
        argv[i] = NULL;


        /* Is command empty */
        if (argv[0] == NULL)
            continue;
        

        /* Does command line end with & */ 
        if (! strcmp(argv[i - 1], "&")) {
            amper = 1;
            argv[i - 1] = NULL;
        }
        else 
            amper = 0; 

        /* Check if the command is for do the */
        if (strcmp(argv[0], "!!") == 0) {  
            char tmp[MAX_COMMAND_SIZE] = "\0";
            if(i > 1){
                for(int index = 1; index< i;index++){
                    strcat(tmp,argv[index]);
                    strcat(tmp," ");
                }    
            }

            strcpy(command, last_cmd);
            int j = 0;
            token = strtok(command, " ");
            while (token != NULL) {
                argv[j] = token;
                token = strtok(NULL, " ");
                j++;
            }

            if(i == 1){
                printf("%s\n", last_cmd);
            }
            else
                printf("%s %s\n",last_cmd,tmp);

            token = strtok(tmp, " ");
            while (token != NULL) {
                argv[j] = token;
                token = strtok(NULL, " ");
                j++;
            }
            argv[j] = NULL;
            i = j;  
        }
        if(i > 2){
            redct(argv,outfile,&i,&redirect);
        }
        else
            redirect = 0;


        /*Check if the command is if and else*/
        if (!strcmp(argv[0], "if")){
            if_then_else(last_cmd,then_else_cmd,&t_flag,&e_flag,i);
            continue;
        }
        
        /* Check if the command is for changing the prompt */
        if (strcmp(argv[0], "prompt") == 0 &&  i == 3 && strcmp(argv[1], "=") == 0) {
            strcpy(prompt,argv[2]);
            continue;
        }
        else if(! (strcmp(argv[0], "prompt"))){
            printf("Commend: prompt = <myprompt>\n");
            continue;
        }

        //read command
        if(!strcmp(argv[0],"read")){
            read_var(argv,varbuff,&varcount,i);
            continue;
        }

        //echo command
        if( argv[0] != NULL && ! strcmp(argv[0], "echo")){  
            for (int idx = 1; idx < i; idx++)
            {
                if(!strcmp(argv[idx],"$?"))   {   
                    sprintf(argv[idx],"%d",status);
                }
                else if (argv[idx][0] == '$'){
                    argv[idx]=get_vval(argv[idx],varbuff,varcount);
                }
            }                      
        }

        //cd command
        if (strcmp(argv[0], "cd") == 0) {
            cd_dir(argv,i);
            continue;
        }

        /* for commands not part of the shell command language */ 
        if (fork() == 0) {
            signal(SIGINT,SIG_DFL); 

            /* redirection of IO ? */
            if (redirect == 1) { // ">"
                fd = creat(outfile, 0660); 
                close (STDOUT_FILENO) ; 
                dup(fd);  
                close(fd); 
                /* stdout is now redirected */
            }
            if(redirect == 2){ // "2>"
                fd = creat(outfile, 0660); 
                close (STDERR_FILENO) ; 
                dup(fd); 
                close(fd); 
                /* stdout is now redirected */
            }
            if (redirect == 3){ // ">>"
                fd = open(outfile, O_WRONLY | O_APPEND | O_CREAT, 0660);
                close (STDOUT_FILENO) ; 
                dup(fd); 
                close(fd); 
                /* stdout is now redirected */
            }

            int statusc = execvp(argv[0], argv);
            if(statusc == -1){
                perror("Unvalid command");
                exit(1);
            }
        }
        

        /* parent continues here */
        if (amper == 0)
            retid = wait(&status);
    }
}
