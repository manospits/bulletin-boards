#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "board_helpfuncs.h"
#include "boardb.h"
#include "element.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "board_errors.h"
#define INP_BUFF_SIZE 256
#define MAX_MSG_SIZE 512
#define CHAN_TTL_S 128
#define HEADER_SIZE_INFO 10

typedef struct{
    char title[CHAN_TTL_S];
    int id;
}channel;

typedef channel * pch;

int chcmp(const void *a,const void *b){
    return strcmp(((pch) a)->title,((pch) b)->title);
}

int main(int argc,char *argv[]){
    //IGNORING CTRL-C SIGNAL
    static  struct  sigaction  act;
    act.sa_handler=SIG_IGN;
    sigfillset (&(act.sa_mask));
    sigaction(SIGINT , &act , NULL);
    //VARIBALES
    char fn[4][128];
    int fd[4];
    int run_flag=1,pid;
    char buff[128];
    if(argc !=2){
        fprintf(stderr,"Wrong call of program board.\n");
        exit(WRONG_CALL);
    }
    FILE *runs;
    //CHECKING IF SERVER RUNS
    sprintf(buff,"%s_spid",argv[1]);
    if((runs=fopen(buff,"r"))==NULL){
        /*puts("_spid file");*/
        run_flag=0;
    }
    else{
        fscanf(runs,"%d",&pid);
        fclose(runs);
        sprintf(buff,"/proc/%d/comm",pid);
        if((runs=fopen(buff,"r"))==NULL){
            /*puts("procs");*/
            run_flag=0;
        }
        else{
            char pname[64];
            fscanf(runs,"%s",pname);
            /*printf("%s \n",pname);*/
            rmn(pname);
            if (strcmp(pname,"board_server")!=0){
                //SERVER DOESNT RUN
                puts("name");
                run_flag=0;
            }
            fclose(runs);
        }
    }
    //OPENING SERVER IF IT DOESNT RUN
    if(run_flag==0){
        if(fork()==0){
            execl("./board_server","board_server",argv[1],NULL);
        }
    }
    sleep(1);
    strcpy(fn[0],argv[1]);
    strcat(fn[0],"self_r");
    strcpy(fn[1],argv[1]);
    strcat(fn[1],"self_w");
    if((fd[1]=open(fn[0],O_WRONLY))<0){
        perror("Error opening fifo");
    }
    //WHILE VARIABLES
    int state=1;
    char inpBuff[INP_BUFF_SIZE];
    char delimeter[]=" ";
    int args[3];
    char *token;
    int arg_count;
    int fail;
    char tmpname[CHAN_TTL_S];
    char msg[MAX_MSG_SIZE];
    char tmpmsg[MAX_MSG_SIZE];
    char *st;
    char tmpheadersize[11],header[128],full;
    int msgsize,final=0;
    sprintf(tmpmsg,"OS");
    sprintf(msg,"%10d",(int)strlen(tmpmsg));
    strcat(msg,tmpmsg);
    if(write(fd[1],msg,strlen(msg))<0){
        perror("Error writing");
        exit(WRITE_ERROR);
    }
    sleep(1);
    if((fd[0]=open(fn[1],O_RDONLY))<0){
        perror("Error opening fifo");
    }
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
        if(strcmp(token,"createchannel")==0){
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
                        strcpy(tmpname,inpBuff+strlen(token)+strlen("createchannel")+2);
                    }
                }
                arg_count++;
            }
            if(arg_count >2&&fail!=1){
                rmn(tmpname);
                if(strlen(tmpname)<CHAN_TTL_S){
                    //creating header
                    sprintf(tmpmsg,"CR %d %s",args[0],tmpname);
                    sprintf(msg,"%10d",(int)strlen(tmpmsg));
                    strcat(msg,tmpmsg);
                    //sending
                    if(write(fd[1],msg,strlen(msg))<0){
                         perror("Write error");
                         exit(WRITE_ERROR);
                    }
                    /*printf("Message sent %s\n",msg);*/
                }
                else{
                    printf("Title too big\n");
                }
            }
            else if(fail!=1){
                puts("Failure: Wrong number of arguments. -createchannel <id> <title>");
            }
        }
        else if(strcmp(token,"getmessages")==0){
            while( token != NULL ){
                token = strtok(NULL, delimeter);
                if(token != NULL && arg_count < 2){
                    if(!isnumber(token)){
                        fail=1;
                        printf("Failure: argument 1 is not a number.\n");
                        break;
                    }
                    if(arg_count==0){
                        args[arg_count]=atoi(token);
                    }
                }
                arg_count++;
            }
            if(fail!=1 && arg_count==2){
                sprintf(tmpmsg,"GM%d",args[0]);
                sprintf(msg,"%10d",(int)strlen(tmpmsg));
                strcat(msg,tmpmsg);
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
                    perror("Error Reading");
                    exit(READ_FAIL);
                }
                header[atoi(tmpheadersize)]='\0';
                /*printf("message received %s\n",header);*/
                sscanf(header,"%d %c",&msgsize,&full); //full flag used to know id there are more messages
                if(full=='y'){
                    if(read(fd[0],tmpmsg,msgsize)<0){
                        perror("Error reading");
                        exit(READ_FAIL);
                    }
                    tmpmsg[msgsize]='\0';
                    printf("--------Received Messages-------\n%s",tmpmsg);
                }
                else if(full=='n'){
                    int full_size;
                    char *list;
                    int last=0;
                    sscanf(header,"%d %c %d",&msgsize,&full,&full_size);
                    if((list = malloc(full_size+1))==NULL){
                         perror("Malloc failed in boardpost.");
                         exit(ALLOCATION_ERROR);
                    }
                    *list='\0';
                    if(read(fd[0],tmpmsg,msgsize)<0){
                        perror("Error reading");
                        exit(READ_FAIL);
                    }
                    tmpmsg[msgsize]='\0';
                    strcat(list,tmpmsg);
                    //loop to get all messages
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
                //getting files server sends them separately
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
                /*printf("message received %s\n",header);*/
                sscanf(header,"%d %c",&msgsize,&full);
                if(full=='y'){
                    int offset=0;
                    if(read(fd[0],tmpmsg,msgsize)<0){
                        perror("Error reading");
                        exit(READ_FAIL);
                    }
                    while(msgsize>0){
                        int  filesize,namesize,fdtocreat,size2wrt;
                        char nums[11],filename[64];
                        memcpy(nums,tmpmsg+offset,10);
                        nums[10]='\0';
                        filesize=atoi(nums);
                        memcpy(nums,tmpmsg+offset+10,10);
                        nums[10]='\0';
                        namesize=atoi(nums);
                        memcpy(filename,tmpmsg+20+offset,namesize);
                        filename[namesize]='\0';
                        printf("file --------%s %d\n",filename,filesize-10-namesize);
                        if((fdtocreat=open(filename,O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR))<0){
                            perror("Open failed");
                            exit(OPEN_FAILURE);
                        }
                        size2wrt=filesize-10-namesize;
                        msgsize=msgsize-filesize-10;
                        if(write(fdtocreat,tmpmsg+20+namesize+offset,size2wrt)<0){
                            perror("Error writing");
                            exit(WRITE_ERROR);
                        }
                        offset+=filesize+10;
                        close(fdtocreat);
                    }
                }
                else if(full=='n'){
                    int full_size;
                    char *list;
                    int last=0,offset=0;
                    sscanf(header,"%d %c %d",&msgsize,&full,&full_size);
                    if((list = malloc(full_size+1))==NULL){
                         perror("Malloc failed in boardpost.");
                         exit(ALLOCATION_ERROR);
                    }
                    if(read(fd[0],tmpmsg,msgsize)<0){
                        perror("Reading failed");
                        exit(READ_FAIL);
                    }
                    memcpy(list,tmpmsg,msgsize);
                    offset=msgsize;
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
                        sscanf(header,"%d %c",&msgsize,&full);
                        if(full=='y'){
                            last=1;
                        }
                        if(read(fd[0],tmpmsg,msgsize)<0){
                            perror("Error reading");
                            exit(READ_FAIL);
                        }
                        memcpy(list+offset,tmpmsg,msgsize);
                        offset+=msgsize;
                    }
                    msgsize=full_size;
                    /*printf("full size %d\n",full_size);*/
                    offset=0;
                    while(offset !=msgsize){
                        int  filesize,namesize,fdtocreat,size2wrt;
                        char nums[11],filename[64];
                        memcpy(nums,list+offset,10);
                        nums[10]='\0';
                        filesize=atoi(nums);
                        memcpy(nums,list+offset+10,10);
                        nums[10]='\0';
                        namesize=atoi(nums);
                        memcpy(filename,list+20+offset,namesize);
                        filename[namesize]='\0';
                        printf("file --------%s %d\n",filename,filesize);
                        if((fdtocreat=open(filename,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR))<0){
                            perror("Open failed");
                            exit(OPEN_FAILURE);
                        }
                        size2wrt=filesize-10-namesize;
                        if(write(fdtocreat,list+20+namesize+offset,size2wrt)<0){
                            perror("Error writing");
                            exit(WRITE_ERROR);
                        }
                        offset=offset+filesize+10;
                        close(fdtocreat);
                    }
                    free(list);
                }
            }
            else if(fail!=1){
                puts("Error : missing arguments- getmessages <id>");
            }
        }
        else if(strcmp(token,"shutdown")==0){
                sprintf(tmpmsg,"SH");
                sprintf(msg,"%10d",(int)strlen(tmpmsg));
                strcat(msg,tmpmsg);
                if(write(fd[1],msg,strlen(msg))<0){
                    perror("Error writing");
                    exit(WRITE_ERROR);
                }
                /*sleep(1);*/
                state=0;
        }
        else if(strcmp(token,"exit")==0){
            sprintf(tmpmsg,"ES");
            sprintf(msg,"%10d",(int)strlen(tmpmsg));
            strcat(msg,tmpmsg);
            if(write(fd[1],msg,strlen(msg))<0){
                perror("Error writing");
                exit(WRITE_ERROR);
            }
            state=0;
        }
        else{
            printf("Error unknown command.\n");
            continue;
        }
    }
    if(final==1){
        sprintf(tmpmsg,"ES");
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
