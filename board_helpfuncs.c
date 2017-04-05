#ifndef _BRDHELP_
#define _BRDHELP_
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define H_SIZE

void cr_header(char * a,int fstmsgtm,char type,int id,int pid,int mid,int pt,char f, int pt_size, int msg_size){
    sprintf(a,"%c %d %d %d %d %d %c %d %d",type,fstmsgtm,id,pid,mid,pt,f,pt_size,msg_size);
}

int isnumber(char *a) {
   char* b = NULL;
   strtol(a, &b, 0);
   return a != NULL && *b == (char)0;
}

void rmn(char * a){
    int i=0;
    while(a[i]!='\0'){
        if(a[i]=='\n'){
            a[i]='\0';
            break;
        }
        i++;
    }
}

int count_spaces(char* a){
    int counter=0;
    int i=0;
    while(a[i]!='\0'){
        if(a[i]==' '){
            counter++;
        }
        i++;
    }
    return counter;

}

#endif
