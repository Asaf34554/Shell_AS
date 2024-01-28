#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include "shell.h"

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


