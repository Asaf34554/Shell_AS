#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include "shell.h"
#include <signal.h>



#define MAX_COMMAND_SIZE 1024


void divide_cmd(char **cmdbuffer, int *n, char *cmd, const char* ch)
{
    char *tkn;
    tkn = strtok(cmd, ch);
    int ndex = -1;
    while (tkn)
    {
        cmdbuffer[++ndex] = malloc(sizeof(tkn) + 1);
        strcpy(cmdbuffer[ndex], tkn);
        unspace(cmdbuffer[ndex]);
        tkn = strtok(NULL, ch);
    }
    cmdbuffer[++ndex] = NULL;
    *n = ndex;
}

void unspace(char* cmd){
    if(cmd[0] == ' ' || cmd[0] == '\n'){
        memmove(cmd,cmd+1,strlen(cmd));
    }
    if(cmd[strlen(cmd)-1] == ' ' || cmd[strlen(cmd)-1] == '\n'){
        cmd[strlen(cmd)-1] = '\0';
    }
}

void pipe_hndl(char **cmdbuf, int index){
    if (index > 50)
    {
        return;
    }

    int fd[50][2]; // 50 pipes
    int indx;
    char *cmd[50];

    for (int i = 0; i < index; i++)
    {
        divide_cmd(cmd, &indx, cmdbuf[i], " ");

        if (i != index - 1){

            if (pipe(fd[i]) == -1){
                perror("ERROR IN PIPE!\n");
                return;
            }
        }
        /*child*/
        if (fork() == 0)
        {
            if (i != index - 1)
            {
                dup2(fd[i][1], STDOUT_FILENO);
                close(fd[i][0]);
                close(fd[i][1]);
            }

            if (i != 0)
            {
                dup2(fd[i - 1][0], STDIN_FILENO);
                close(fd[i - 1][1]);
                close(fd[i - 1][0]);
            }
            execvp(cmd[0], cmd);
            exit(1);
        }
        /*parent*/
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
void cd_dir(char* argv [],int len){
    // char* ans;
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
        char dir[1024] = "";
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
            // perror("cd:");
        }
    }

}

void sig_int(int sig){
    printf("\nYou typed Control-C!\n");
}



int main() {
    char command[MAX_COMMAND_SIZE];
    char last_cmd[MAX_COMMAND_SIZE];
    char pipe_cmd[MAX_COMMAND_SIZE];
    char *token;
    char outfile[1024];
    int i, fd, amper, redirect, retid, status,id_len;
    char *argv[10];
    char *cmdbuffer[256];

    char prompt[256];
    strcpy(prompt,"Maccabi Haifa");



    // Handles CTRL+C Signal and ignores it
    signal(SIGINT,sig_int);
    while (1)
    {   
        printf("%s: ",prompt);    
        fgets(command, MAX_COMMAND_SIZE, stdin);
        command[strlen(command) - 1] = '\0';
        

        if(strncmp(command,"!!",2) ){
            strcpy(last_cmd,command);
        }
        
        if(! strcmp(command,"quit")){
            exit(0);
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
                
                divide_cmd(cmdbuffer, &id_len, pipe_cmd, "|");
                pipe_hndl(cmdbuffer, id_len);
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
            if (!strcmp(argv[i - 2], ">") || !strcmp(argv[i - 2], "2>") || !strcmp(argv[i - 2], ">>")) {
                if (!strcmp(argv[i - 2], ">"))
                    redirect = 1;

                else if(! strcmp(argv[i - 2], "2>"))
                    redirect = 2;

                else // ">>"
                    redirect = 3;
                
                argv[i - 2] = NULL;
                strcpy(outfile,argv[i-1]);
            } 
            else
                redirect = 0;
        }
        else
            redirect = 0;


        /* Check if the command is for changing the prompt */
        if (strcmp(argv[0], "prompt") == 0 && strcmp(argv[1], "=") == 0 && i == 3) {
            strcpy(prompt,argv[2]);
            continue;
        }
        else if(! (strcmp(argv[0], "prompt"))){
            printf("Commend: prompt = <myprompt>\n");
            continue;
        }

        //$? command
        if( argv[1] != NULL && !( strcmp(argv[1], "$?"))){        
            strcpy(argv[1],strerror(status));
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