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

void sig_int(int sig){
    printf("\nYou typed Control-C!\n");
}



int main() {
    char command[MAX_COMMAND_SIZE];
    char last_command[MAX_COMMAND_SIZE];
    char *token;
    char *outfile;
    int i, fd, amper, redirect, retid, status;
    char *argv[10];
    char prompt[256];
    strcpy(prompt,"Maccabi Haifa");
    // Handles CTRL+C Signal and ignores it.
    
    signal(SIGINT,sig_int);
    while (1)
    {   
        printf("%s: ",prompt);    
        fgets(command, MAX_COMMAND_SIZE, stdin);
        command[strlen(command) - 1] = '\0';
        

        if(strncmp(command,"!!",2) ){
            strcpy(last_command,command);
        }
        
        if(! strcmp(command,"quit")){
            exit(0);
        }

        /* parse command line */ 
        i = 0;
        token = strtok (command," ");
        while (token != NULL)
        {
            argv[i] = token;
            token = strtok (NULL, " ");
            i++;
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

            strcpy(command, last_command);
            int j = 0;
            token = strtok(command, " ");
            while (token != NULL) {
                argv[j] = token;
                token = strtok(NULL, " ");
                j++;
            }

            if(i == 1){
                printf("%s\n", last_command);
            }
            else
                printf("%s %s\n",last_command,tmp);

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
                outfile = argv[i - 1];
            } 
            else
                redirect = 0;
        }


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
        
        // //saves the last executed command
        // strcpy(last_command,command);
        // printf("last com:%s\ncom:%s\n",last_command,command);
    }
}