#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include "board_helpfuncs.h"
#include "boardb.h"
#include "element.h"
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include "board_errors.h"
#define INP_BUFF_SIZE 512
#define MAX_MSG_SIZE 512
#define CHAN_TTL_S 128

typedef struct{
    char title[CHAN_TTL_S];
    int id;
}channel;

typedef channel * pch;

int chcmp(const void *a,const void *b){
    return strcmp(((pch) a)->title,((pch) b)->title);
}

int main(int argc,char *argv[]){
    //IGNORING CTRL-C signal
    static  struct  sigaction  act;
    act.sa_handler=SIG_IGN;
    sigfillset (&(act.sa_mask));
    sigaction(SIGINT , &act , NULL);
    char fn[4][128];
    int fd[4];
    int run_flag=1;
    int pid;
    char buff[128];
    if(argc !=2){
        fprintf(stderr,"Wrong call of program board.\n");
        exit(WRONG_CALL);
    }
    FILE *runs;
    sprintf(buff,"%s_spid",argv[1]);
    if((runs=fopen(buff,"r"))==NULL){
        puts("_spid file");
        run_flag=0;
    }
    else{
        fscanf(runs,"%d",&pid);
        fclose(runs);
        sprintf(buff,"/proc/%d/comm",pid);
        if((runs=fopen(buff,"r"))==NULL){
            puts("procs");
            run_flag=0;
        }
        else{
            char pname[64];
            fscanf(runs,"%s",pname);
            if (strcmp(pname,"board_server")!=0){
                //SERVER DOESNT RUN
                puts("name");
                run_flag=0;
            }
            fclose(runs);
        }
    }
    if(run_flag==0){
        fprintf(stderr,"Server is not running.");
        exit(SERVER_DOWN_FAIL);
    }
    /*if((runs=fopen(buff,"r"))==NULL){*/
        /*perror("Error opening file");*/
        /*exit(OPEN_FAILURE);*/
    /*}*/
    /*fscanf(runs,"%d",&pid);*/
    /*fclose(runs);*/
    strcpy(fn[0],argv[1]);
    strcat(fn[0],"others_r");
    strcpy(fn[1],argv[1]);
    strcat(fn[1],"others_w");
    /*kill(pid,SIGUSR2);*/
    if((fd[1]=open(fn[0],O_WRONLY))<0){
        perror("Error opening fifo");
    }
    //WHILE VARIABLES
    int state=1;
    char inpBuff[INP_BUFF_SIZE],tmpname[CHAN_TTL_S], tmpheadersize[11],header[128];
    char msg[MAX_MSG_SIZE],tmpmsg[MAX_MSG_SIZE],tmpmsg2[MAX_MSG_SIZE],*st,full;
    char delimeter[]=" ",*token;
    int args[3],arg_count,fail;
    int msgsize,messagesid=0;
    sprintf(tmpmsg,"OO");
    sprintf(msg,"%10d",(int)strlen(tmpmsg));
    strcat(msg,tmpmsg);
    if(write(fd[1],msg,strlen(msg))<0){
        perror("Write failed");
        exit(WRITE_ERROR);
    }
    if((fd[0]=open(fn[1],O_RDONLY))<0){
        perror("Error opening fifo");
    }
    sleep(1);
    int final=0;
    while(state){
        printf(">");
        st=fgets(inpBuff,INP_BUFF_SIZE,stdin);
        if(st==NULL){
            final=1;
            break;
        }
        rmn(inpBuff);
        arg_count=0;
        fail=0;
        if(strlen(inpBuff)<1){
            continue;
        }
        if(count_spaces(inpBuff)==strlen(inpBuff)){
            continue;
        }
        token=strtok(inpBuff,delimeter);
        rmn(token);
        if(strcmp(token,"write")==0){
            while( token != NULL ){
                token = strtok(NULL, delimeter);
                if(token != NULL && arg_count < 2){
                    if(!isnumber(token)&& arg_count==0){
                        fail=1;
                        printf("Failure: argument 1 is not a number.\n");
                        continue;
                    }
                    if(arg_count==0){
                        args[arg_count]=atoi(token);
                        strcpy(tmpname,inpBuff+strlen(token)+strlen("write")+2);
                    }
                }
                arg_count++;
            }
            /*printf("%d\n",arg_count);*/
            if(arg_count >2 &&fail!=1){
                rmn(tmpname);
                if(strlen(tmpname)<CHAN_TTL_S){
                    //creating header
                    sprintf(tmpmsg,"WR");
                    cr_header(tmpmsg2,time(NULL),'w',args[0],getpid(),messagesid++,0,'y',strlen(tmpname),strlen(tmpname));
                    strcat(tmpmsg,tmpmsg2);
                    sprintf(msg,"%10d",(int)strlen(tmpmsg));
                    strcat(msg,tmpmsg);
                    strcat(msg,tmpname);
                    /*printf("%d\n",fd[1]);*/
                    if(write(fd[1],msg,strlen(msg))<0){
                        perror("Error writing");
                        exit(WRITE_ERROR);
                    }
                    /*printf("Message sent %s\n",msg);*/
                }
                else{
                    printf("Message to big\n");
                }
            }
            else if(fail!=1){
                 puts("Failure: Wrong number of arguments.");
            }
        }
        else if(strcmp(token,"list")==0){
            sprintf(tmpmsg,"LI");
            sprintf(msg,"%10d",(int)strlen(tmpmsg));
            strcat(msg,tmpmsg);
            /*printf("%d\n",fd[1]);*/
            if(write(fd[1],msg,strlen(msg))<0){
                perror("Error writing");
                exit(WRITE_ERROR);
            }
            if(read(fd[0],tmpheadersize,HEADER_SIZE_INFO)<0){
                perror("Error reading");
                exit(READ_FAIL);
            }
            tmpheadersize[10]='\0';
            /*printf("header size:%s\n",tmpheadersize);*/
            if(read(fd[0],header,atoi(tmpheadersize))<0){
                perror("Error reading");
                exit(READ_FAIL);
            }
            header[atoi(tmpheadersize)]='\0';
            sscanf(header,"%d %c",&msgsize,&full);
            if(full=='y'){
                if(read(fd[0],tmpmsg,msgsize)<0){
                    perror("Error reading");
                    exit(READ_FAIL);
                }
                tmpmsg[msgsize]='\0';
                printf("%s",tmpmsg);
            }
            else if(full=='n'){
                int full_size,last=0;
                char *list;
                sscanf(header,"%d %c %d",&msgsize,&full,&full_size);
                if((list = malloc(full_size+1))==NULL){
                    perror("Malloc failed in boardpost.");
                    exit(ALLOCATION_ERROR);
                }
                *list='\0';
                if( read(fd[0],tmpmsg,msgsize)<0){
                    perror("Error reading");
                    exit(READ_FAIL);
                }
                tmpmsg[msgsize]='\0';
                strcat(list,tmpmsg);
                /*printf("messages %s",list);*/
                while(!last){
                    if(read(fd[0],tmpheadersize,HEADER_SIZE_INFO)<0){
                        perror("Error reading");
                        exit(READ_FAIL);
                    }
                    tmpheadersize[10]='\0';
                    /*printf("header size:%s\n",tmpheadersize);*/
                    if(read(fd[0],header,atoi(tmpheadersize))<0){
                        perror("Error reading");
                        exit(READ_FAIL);
                    }
                    /*printf("header:%s\n",header);*/
                    sscanf(header,"%d %c",&msgsize,&full);
                    if(full=='y'){
                         last=1;
                    }
                    if(read(fd[0],tmpmsg,msgsize)<0){
                        perror("Error reading");
                        exit(READ_FAIL);
                    }
                    tmpmsg[msgsize]='\0';
                    strcat(list,tmpmsg);
                }
                printf("%s",list);
                free(list);
            }
        }
        else if(strcmp(token,"send")==0){
            while( token != NULL ){
                token = strtok(NULL, delimeter);
                if(token != NULL && arg_count < 2){
                    if(!isnumber(token)&& arg_count==0){
                        fail=1;
                        printf("Failure: argument 1 is not a number.\n");
                        continue;
                    }
                    if(arg_count==0){
                        args[arg_count]=atoi(token);
                    }
                    if(arg_count==1){
                        strcpy(tmpname,token);
                    }
                }
                arg_count++;
            }
            if(arg_count==3){
                struct stat file2sendinfo;
                if(stat(tmpname,&file2sendinfo)<0){
                    printf("Error accessing file %s\n",tmpname);
                }
                else{
                    msgsize=file2sendinfo.st_size;
                    int openedfile,nread,first;
                    if((openedfile=open(tmpname,O_RDONLY))<0){
                        puts("Error opening file");
                    }
                    char ndlmtr[]="/ ";
                    char *name;
                    char filename[64];
                    first=1;
                    name=strtok(tmpname,ndlmtr);
                    while(name!=NULL){
                        strcpy(filename,name);
                        name=strtok(NULL,ndlmtr);
                    }
                    char* buffer[MAX_MSG_SIZE-128];
                    int size2read=MAX_MSG_SIZE-128,size2write,fsttime=time(NULL),pt_size;
                    sprintf(tmpname,"%10d%s",(int)strlen(filename),filename);
                    /*printf("%d %d %s\n",msgsize,(int)strlen(tmpname),tmpname);*/
                    size2read=size2read-strlen(tmpname);
                    msgsize+=strlen(tmpname);
                    while ( (nread=read(openedfile , buffer , size2read) ) > 0 ){
                        sprintf(tmpmsg,"WR");
                        if(first==1){
                            pt_size=strlen(tmpname)+nread;
                        }
                        else{
                            pt_size=nread;
                        }
                        if(nread<size2read)
                            cr_header(tmpmsg2,fsttime,'f',args[0],getpid(),messagesid,0,'y',pt_size,msgsize);
                        else{
                            cr_header(tmpmsg2,fsttime,'f',args[0],getpid(),messagesid,0,'n',pt_size,msgsize);
                        }
                        strcat(tmpmsg,tmpmsg2);
                        sprintf(msg,"%10d",(int)strlen(tmpmsg));
                        strcat(msg,tmpmsg);
                        if(first==1){
                            first=0;
                            size2write=strlen(msg)+nread+strlen(tmpname);
                            size2read+=strlen(tmpname);
                            strcat(msg,tmpname);
                        }
                        else{
                            size2write=strlen(msg)+nread;
                        }
                        memcpy(msg+strlen(msg),buffer,nread);
                        if(write(fd[1],msg,size2write)<0){
                            perror("Error writing");
                            exit(WRITE_ERROR);
                        }
                    }
                    close(openedfile);
                    messagesid++;
                }
            }
        }
        else if(strcmp(token,"exit")==0){
            sprintf(tmpmsg,"EO");
            sprintf(msg,"%10d",(int)strlen(tmpmsg));
            strcat(msg,tmpmsg);
            /*printf("%d\n",fd[1]);*/
            if(write(fd[1],msg,strlen(msg))<0){
                 perror("Error writing");
                 exit(WRITE_ERROR);
            }
            /*printf("Message sent %s\n",msg);*/
            /*kill(pid,SIGUSR1);*/
            state=0;
        }
        else{
            printf("Error unknown command.\n");
            continue;
        }
    }
    if(final==1){
        sprintf(tmpmsg,"EO");
        sprintf(msg,"%10d",(int)strlen(tmpmsg));
        strcat(msg,tmpmsg);
        if(write(fd[1],msg,strlen(msg))<0){
            perror("Error writing");
            exit(WRITE_ERROR);
        }
    }
    close(fd[0]);
    close(fd[1]);
    return 0;
}
