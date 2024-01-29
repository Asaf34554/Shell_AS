#ifndef SHELL_H
#define SHELL_H

void cd_dir(char*[],int);
void sig_int(int);
void divide_cmd(char**,int*,char*,const char*);
void unspace(char*);
void pipe_hndl(char **, int );



#endif

