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
    if(len == 1){
        if(chdir("~") != 0){
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
            strcat(dir," ");
        }
        dir[strlen(dir) - 1] = '\0';
        if(chdir(dir) != 0){
            perror("cd");
        }
    }

}


