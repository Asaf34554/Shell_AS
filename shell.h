#ifndef SHELL_H
#define SHELL_H

typedef struct _VAR{
    char vname [256];
    char  vvalue[256];
}vari;


void redct(char**,char*,int*,int*);
void if_then_else(char*,char*,int*,int*,const int);
void add_var(char*,vari*,int*);
char* get_vval(char*,vari*,const int);
void cd_dir(char*[],int);
void sig_int(int);
void divide_cmd(char**,int*,char*,const char*);
void unspace(char*);
void pipe_hndl(char **, int );
void read_var(char**,vari*,int*,const int);


#endif

