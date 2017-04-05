#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "boardb.h"
#include "element.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "board_errors.h"
#include "board_helpfuncs.h"
#define CHAN_TTL_S 128

int board_flag=0,boardpost_flag=0;

void board_hndlr(int signum){
    if(signum==SIGUSR1){
        puts("received 1");
        board_flag=1;
    }
}

void boardp_hndlr(int signum){
    if(signum==SIGUSR2){
        puts("received 2");
        boardpost_flag=1;
    }
}

typedef struct{
    char title[CHAN_TTL_S];
    int id;
}channel;


void ch_print(void *a,char *buff){
    channel *aa=(channel *)a;
    sprintf(buff,"%d %s\n",aa->id,aa->title);
    /*printf("-------%d %s\n",aa->id,aa->title);*/
}

void ch_pr(void *a){
    channel *aa=(channel *)a;
    printf("-------%d %s\n",aa->id,aa->title);
}

typedef channel * pch;

int chcmp(const void *a,const void *b){
    return intcmp(&((pch) a)->id,&((pch) b)->id);
}

int main(int argc,char *argv[]){
    static struct sigaction act1,act2;
    act1.sa_handler=board_hndlr;    // the  handler  is set to  IGNORE
    sigfillset (&(act1.sa_mask));
    sigaction(SIGUSR1 , &act1 , NULL);
    act2.sa_handler=boardp_hndlr;    // the  handler  is set to  IGNORE
    sigfillset (&(act2.sa_mask));
    sigaction(SIGUSR2 , &act2 , NULL);
    char fn[4][128];
    int fd[4],i,fstat;
    char buff[128],header[128];
    char tmpmsg[MSG_SIZE],tmpmsg2[MSG_SIZE];
    fd_set read_fds;
    /*puts("server call\n");*/
    if(argc !=2){
        fprintf(stderr,"Wrong call of program board.\n");
        exit(WRONG_CALL);
    }
    /*printf("Server call %s",argv[1]);*/
    FILE *runs;
    sprintf(buff,"%s_spid",argv[1]);
    runs=fopen(buff,"w");
    fprintf(runs,"%d",getpid());
    fclose(runs);
    pel_info string;
    string=create_type(sizeof(channel),chcmp);
    pbboard aboard;
    aboard=cr_bboard(string);
    strcpy(fn[0],argv[1]);
    strcat(fn[0],"self_r");
    if((fstat=mkfifo(fn[0],0666))<0){
        perror("Creating fifo failed.\n");
        exit(FIFO_CREAT_FAIL);
    }
    strcpy(fn[1],argv[1]);
    strcat(fn[1],"self_w");
    if(mkfifo(fn[1],0666)==-1){
        perror("Creating fifo failed.\n");
        exit(FIFO_CREAT_FAIL);
    }
    strcpy(fn[2],argv[1]);
    strcat(fn[2],"others_r");
    if(mkfifo(fn[2],0666)==-1){
        perror("Creating fifo failed.\n");
        exit(FIFO_CREAT_FAIL);
    }
    strcpy(fn[3],argv[1]);
        strcat(fn[3],"others_w");
    if(mkfifo(fn[3],0666)==-1){
        perror("Creating fifo failed.\n");
        exit(FIFO_CREAT_FAIL);
    }
    for(i=0;i<4;i++){
        if(i%2==0){
                if((fd[i]=open(fn[i],O_RDWR|O_NONBLOCK))<0){
                    perror("Server Error : Error opening fifo\n");
                    exit(FIFO_OPEN_FAIL);
                 }
        }
    }
    int state=1;
    int boardpostnum=0;
    int tmpfd;
    char opt[3];
    char tmpheadersize[11];
    char tmpheader[128];
    while(state){
        /*puts("server loop");*/
        FD_ZERO(&read_fds);
        /*if (board_flag)*/
        /*if(board_flag==1)*/
        FD_SET(fd[0],&read_fds);
        /*if(boardpost_flag==1)*/
        FD_SET(fd[2],&read_fds);
        if(select(FD_SETSIZE,&read_fds,NULL,NULL,NULL)<0){
            perror("Server Error: select failed\n");
            /*continue;*/
            exit(SELECT_FAIL);
        }
        for(i=0;i<FD_SETSIZE;i++){
            if(FD_ISSET(i,&read_fds)){
                tmpfd=i;
                if(read(tmpfd,tmpheadersize,HEADER_SIZE_INFO)<0){
                     perror("Error Reading");
                     exit(READ_FAIL);
                }
                tmpheadersize[10]='\0';
                if(read(tmpfd,tmpheader,atoi(tmpheadersize))<0){
                    perror("Error Reading");
                    exit(READ_FAIL);
                }
                tmpheader[atoi(tmpheadersize)]='\0';
                /*printf("Message in server %d%s %d\n",atoi(tmpheadersize),tmpheader,tmpfd);*/
                strncpy(opt,tmpheader,2);
                opt[2]='\0';
                if(strcmp(opt,"CR")==0){
                    int id,count;
                    char name[CHAN_TTL_S];
                    channel tmpchannel;
                    i=0;
                    count=0;
                    sscanf(tmpheader,"%s %d",tmpheadersize,&id);
                    while(count!=2){
                        if(tmpheader[i++]==' '){
                            count++;
                        }
                    }
                    tmpchannel.id=id;
                    strcpy(name,tmpheader+i);
                    strcpy(tmpchannel.title,name);
                    ginsert(aboard,&tmpchannel);
                }
                else if(strcmp(opt,"SH")==0){
                     state=0;
                }
                else if(strcmp(opt,"OS")==0){
                    if((fd[1]=open(fn[1],O_WRONLY))<0){
                        /*printf("%d %s\n",1,fn[1]);*/
                        perror("Server Error : Error opening fifo\n");
                        exit(FIFO_OPEN_FAIL);
                    }
                }
                else if(strcmp(opt,"OO")==0){
                    if(boardpostnum==0){
                        if((fd[3]=open(fn[3],O_WRONLY))<0){
                            perror("Server Error : Error opening fifo\n");
                            exit(FIFO_OPEN_FAIL);
                        }
                    }
                    boardpostnum++;
                }
                else if(strcmp(opt,"EO")==0){
                    boardpostnum--;
                    if(boardpostnum==0){
                        close(fd[3]);
                        boardpost_flag=0;
                    }
                }
                else if(strcmp(opt,"ES")==0){
                    sleep(1);
                    close(fd[1]);
                    board_flag=0;
                }
                else if(strcmp(opt,"LI")==0){
                    char *names;
                    names=get_channel_names(aboard,ch_print);
                    int sizeofnames=MSG_SIZE-strlen(names)-15;
                    if(sizeofnames>=0){
                        sprintf(header,"%3d y",(int)strlen(names));
                        sprintf(tmpmsg,"%10d%s%s",(int)strlen(header),header,names);
                        if(write(fd[3],tmpmsg,strlen(tmpmsg))<0){
                            perror("Error Writing");
                            exit(WRITE_ERROR);
                        }
                        /*printf("msg2sent :%s",tmpmsg);*/
                        free(names);
                    }
                    else{
                        int endofmessage=1;
                        int offset=0,rd;
                        while(endofmessage){
                            strncpy(tmpmsg2,names+offset,MSG_SIZE-64);
                            if(strlen(names+offset)>MSG_SIZE-64){
                                tmpmsg2[MSG_SIZE-64]='\0';
                                rd=MSG_SIZE-64;
                            }
                            else{
                                rd=strlen(names+offset);
                            }
                            if(strlen(tmpmsg2)<MSG_SIZE-64)
                                endofmessage=0;
                            offset+=rd;
                            if(endofmessage==0)
                                sprintf(header,"%3d y",(int)strlen(tmpmsg2));
                            else{
                                sprintf(header,"%3d n %3d",(int)strlen(tmpmsg2),(int)strlen(names));
                            }
                            sprintf(tmpmsg,"%10d%s%s",(int)strlen(header),header,tmpmsg2);
                            /*printf("%s\n",tmpmsg);*/
                            if(write(fd[3],tmpmsg,strlen(tmpmsg))<0){
                                perror("Error Writing");
                                exit(WRITE_ERROR);
                            }
                        }
                        free(names);
                    }
                }
                else if(strcmp(opt,"WR")==0){
                    int mid,pid,time,id,pt,pt_size,msg_size;
                    char type,flag;
                    channel tmpchannel;
                    sscanf(tmpheader+2,"%c %d %d %d %d %d %c %d %d",&type,&time,&id,&pid,&mid,&pt,&flag,&pt_size,&msg_size);
                    tmpchannel.id=id;
                    if(read(tmpfd,tmpmsg,pt_size)<0){
                        perror("Reading message failed\n");
                        exit(READ_FAIL);
                    }
                    if(type=='w')
                        tmpmsg[msg_size]='\0';
                    if(flag=='y'){
                        /*printf("final message header came and size of file is %d\n",msg_size);*/
                    }
                    int stat=add_message_d(aboard,&tmpchannel,tmpmsg,time,msg_size ,type,flag,mid,pid,pt_size);
                    if(stat==1){
                        /*puts("error in insert\n");*/
                    }
                }
                else if(strcmp(opt,"GM")==0){
                    int id=atoi(tmpheader+2),filessize,size2write;
                    channel tmp;
                    tmp.id=id;
                    strcpy(tmp.title,"nothing");
                    char *msgs;
                    msgs=get_ready_messages(aboard,&tmp);
                    int sizeofnames=MSG_SIZE-strlen(msgs)-15;
                    if(sizeofnames>=0){
                        sprintf(header,"%3d y",(int)strlen(msgs));
                        sprintf(tmpmsg,"%10d%s%s",(int)strlen(header),header,msgs);
                        if(write(fd[1],tmpmsg,strlen(tmpmsg))<0){
                            perror("Error Writing");
                            exit(WRITE_ERROR);
                        }
                        free(msgs);
                    }
                    else{
                        int endofmessage=1;
                        int offset=0,rd;
                        while(endofmessage){
                            strncpy(tmpmsg2,msgs+offset,MSG_SIZE-64);
                            if(strlen(msgs+offset)>MSG_SIZE-64){
                                tmpmsg2[MSG_SIZE-64]='\0';
                                rd=MSG_SIZE-64;
                            }
                            else{
                                rd=strlen(msgs+offset);
                            }
                            if(strlen(tmpmsg2)<MSG_SIZE-64)
                                endofmessage=0;
                            offset+=rd;
                            if(endofmessage==0){
                                sprintf(header,"%3d y",(int)strlen(tmpmsg2));
                            }
                            else{
                                sprintf(header,"%3d n %3d",(int)strlen(tmpmsg2),(int)strlen(msgs));
                            }
                            sprintf(tmpmsg,"%10d%s%s",(int)strlen(header),header,tmpmsg2);
                            if(write(fd[1],tmpmsg,strlen(tmpmsg))<0){
                                perror("Error Writing");
                                exit(WRITE_ERROR);
                            }
                        }
                        free(msgs);
                    }
                    msgs=get_ready_files(aboard,&tmp,&filessize);
                    /*printf("%d\n",filessize);*/
                    sizeofnames=MSG_SIZE-filessize-15;
                    if(sizeofnames>=0){
                        sprintf(header,"%3d y",filessize);
                        sprintf(tmpmsg,"%10d%s",(int)strlen(header),header);
                        size2write=strlen(tmpmsg)+filessize;
                        memcpy(tmpmsg+strlen(tmpmsg),msgs,filessize);
                        if(write(fd[1],tmpmsg,size2write)<0){
                            perror("Error Writing");
                            exit(WRITE_ERROR);
                        }
                        free(msgs);
                    }
                    else{
                        int endofmessage=1;
                        int offset=0,rd;
                        while(endofmessage){
                            if(filessize-offset<MSG_SIZE-64){
                                endofmessage=0;
                                memcpy(tmpmsg2,msgs+offset,filessize-offset);
                                rd=filessize-offset;
                            }
                            else{
                                memcpy(tmpmsg2,msgs+offset,MSG_SIZE-64);
                                rd=MSG_SIZE-64;
                            }
                            offset+=rd;
                            if(endofmessage==0){
                                sprintf(header,"%3d y",rd);
                            }
                            else{
                                sprintf(header,"%d n %d",rd,filessize);
                            }
                            sprintf(tmpmsg,"%10d%s",(int)strlen(header),header);
                            /*printf("server :%s\n",tmpmsg);*/
                            size2write=strlen(tmpmsg)+rd;
                            memcpy(tmpmsg+strlen(tmpmsg),tmpmsg2,rd);
                            if(write(fd[1],tmpmsg,size2write)<0){
                                perror("Error Writing");
                                exit(WRITE_ERROR);
                            }
                        }
                        free(msgs);
                    }
                }
            }
        }
    }
    free(string);
    for(i=0;i<4;i++){
        if(i==0||i==1)
        close(fd[i]);
        remove(fn[i]);
    }
    /*remove("_spid");*/
    ds_bboard(aboard);
    return 0;
}
