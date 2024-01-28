#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>




int main() {
char command[1024];
char *token;
char *outfile;
int i, fd, amper, redirect, retid, status;
char *argv[10];
char prompt[256];
strcpy(prompt,"Maccabi_Haifa");

while (1)
{
    printf("%s: ",prompt);
    fgets(command, 1024, stdin);
    command[strlen(command) - 1] = '\0';

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

    if (!strcmp(argv[i - 2], ">") || !strcmp(argv[i - 2], "2>") || !strcmp(argv[i - 2], ">>")) {
        if (!strcmp(argv[i - 2], ">"))
            redirect = 1;

        else if(! strcmp(argv[i - 2], "2>"))
            redirect = 2;

        else
            redirect = 3;
        
        argv[i - 2] = NULL;
        outfile = argv[i - 1];
    } 
    else
        redirect = 0;


    /* Check if the command is for changing the prompt */
    if (strcmp(argv[0], "prompt") == 0 && strcmp(argv[1], "=") == 0 && i == 3) {
        printf("inside prompt\n");
        strcpy(prompt,argv[2]);
        continue;
    }
    else if(! (strcmp(argv[0], "prompt"))){
        printf("Commend: prompt = <myprompt>\n");
        continue;
    }


    if( argv[1] != NULL && !( strcmp(argv[1], "$?"))){        
        printf("inside $?\n");

        strcpy(argv[1],strerror(status));
    }



    /* for commands not part of the shell command language */ 
    if (fork() == 0) { 
        /* redirection of IO ? */
        if (redirect == 1) {
            fd = creat(outfile, 0660); 
            close (STDOUT_FILENO) ; 
            dup(fd); 
            close(fd); 
            /* stdout is now redirected */
        }
        if(redirect == 2){
            fd = creat(outfile, 0660); 
            close (STDERR_FILENO) ; 
            dup(fd); 
            close(fd); 
            /* stdout is now redirected */
        }
        if (redirect == 3){
            fd = open(outfile, O_WRONLY | O_APPEND | O_CREAT, 0660);
            close (STDOUT_FILENO) ; 
            dup(fd); 
            close(fd); 
            /* stdout is now redirected */

        }
        
        execvp(argv[0], argv);
    }
    /* parent continues here */
    if (amper == 0)
        retid = wait(&status);
}
}