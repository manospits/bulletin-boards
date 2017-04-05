#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "boardb.h"

typedef struct bboard_info {
    phead channels;
    int num_of_channels;
    pel_info data_info; //information about the data stored in bboard
    pel_info channel_info; //channel_type info
    pel_info message_info;  //arc_type info
} bboard_info;

typedef struct bboard_channel {
    phead messages;
    void* data;
    int in;
    int out;
} bboard_channel;

typedef struct message {
    int mid;
    int pid;
    time_t fst_msg_tm;
    char *msg;
    char type;  //m for message f for file
    char full;   //y if full n if not full
    int msgsize;
    int rcvd;
} message;

pel_info data_info;

void ds_message(pmessage a)
{
    free(a->msg);
    free(a);
}

void message_destr(void * a){
    ds_message((pmessage)a);
}

void channeldestr(void *a){
    type_destr(data_info,((pgchannel)a)->data);
    ds_list_plus_data(((pgchannel)a)->messages,message_destr);
    /*printf("%p\n",a);*/
    free(a);
    /*puts("channel destroyed");*/
}

int gchannel_cmp(const void *a,const void *b){
    pgchannel an=(pgchannel) a;
    pgchannel bn=(pgchannel) b;
    int (*cmp)(const void *,const void*);
    cmp=get_cmp(data_info);
    if(cmp(an->data,bn->data)==0){
        return 0;
    }
    else{
         return 1;
    }
}

int message_cmp(const void *a ,const void * b){
    pmessage aa,bb;
    aa = (pmessage) a;
    bb = (pmessage) b;
    if(aa->mid==bb->mid && aa->pid==bb->pid && aa->type==bb->type && aa->msgsize == bb->msgsize && aa->fst_msg_tm == bb->fst_msg_tm){
         return 0;
    }
    else{
        return 1;
    }
}

int msginboard(pbboard bboardh, void* data,time_t fstmsgtm,size_t msgsize,char type, int mid, int pid){
    pgchannel a=get_gchannel(bboardh,data);
    if(a==NULL){
         return 1;
    }
    message tmp;
    tmp.msgsize=msgsize;
    tmp.type=type;
    tmp.mid=mid;
    tmp.pid=pid;
    tmp.fst_msg_tm=fstmsgtm;
    return msginchannel(a,&tmp);
}

int msginchannel(const pgchannel M,pmessage a){
    return in(M->messages,a);
}

pbboard cr_bboard (pel_info pinfo){
    bboard_info *tmpbboard;
    if((tmpbboard=malloc(sizeof(struct bboard_info)))==NULL){
        fprintf(stderr,"Error allocating space -bboard.c\n");
        exit(1);
    }
    tmpbboard->num_of_channels=0;
    tmpbboard->data_info=pinfo;
    data_info = pinfo;
    tmpbboard->channel_info=create_type(sizeof(struct bboard_channel),&gchannel_cmp);
    /*printf("bboard hash size is %d\n",tmpbboard->hash_size);*/
    tmpbboard->message_info=create_type(sizeof(struct message),&message_cmp);
    tmpbboard->channels=cr_list(tmpbboard->channel_info);
    return tmpbboard;
}

pgchannel cr_gchannel (pbboard bboardh,const void *data){
    bboard_channel* tmpgchannel;
    tmpgchannel=malloc(sizeof(struct bboard_channel));
    if(tmpgchannel==NULL){
        fprintf(stderr,"Error allocating space -bboard.c\n");
        exit(1);
    }
    /*printf("creating channel %p\n",tmpgchannel);*/
    tmpgchannel->messages=cr_list(bboardh->message_info);
    tmpgchannel->in=0;
    tmpgchannel->out=0;
    tmpgchannel->data=malloc(get_type_size(data_info));
    if(tmpgchannel->data==NULL){
        fprintf(stderr,"Error allocating space -bboard.c\n");
        exit(1);
    }
    memcpy(tmpgchannel->data,data,get_type_size(data_info));
    return tmpgchannel;
}

pmessage cr_message(char* msg,time_t fstmsgtm,size_t msgsize,char type,char flag, int mid , int pid , int rcvd){
    message *tmpmessage;
    tmpmessage=malloc(sizeof(struct message));
    tmpmessage->full=flag;
    tmpmessage->msgsize=msgsize;
    tmpmessage->type = type;
    tmpmessage->mid = mid;
    tmpmessage->pid = pid;
    tmpmessage->fst_msg_tm = fstmsgtm;
    tmpmessage->rcvd=rcvd;
    if(tmpmessage==NULL){
        fprintf(stderr,"Error allocating space -bboard.c\n");
        exit(1);
    }
    if((tmpmessage->msg=malloc(msgsize+1))==NULL){
        puts("malloc error");
    }
    memcpy(tmpmessage->msg,msg,rcvd);
    if(type=='w'){
         tmpmessage->msg[msgsize]='\0';
    }
    return tmpmessage;
}

int in_bboard(pbboard bboardh , void *data){
    /*puts("in bboard call");*/
    bboard_channel tmp;
    tmp.data=malloc(get_type_size(data_info));
    if(tmp.data==NULL){
        fprintf(stderr,"Error allocating space -bboard.c\n");
        exit(1);
    }
    memcpy(tmp.data,data,get_type_size(data_info));
    int stat;
    stat=in(bboardh->channels,&tmp);
    free (tmp.data);
    /*printf("stat is %d\n",stat);*/
    return stat;
}

int ginsert(pbboard bboardh,void *data){
    if(in_bboard(bboardh,data)==1){
            return 1;
    }
    pgchannel tmp=cr_gchannel(bboardh,data);
    insert(bboardh->channels,tmp);
    /*printf("%d %s",((channel*)(tmp->data))->id,((channel *)(tmp->data))->title);*/
    bboardh->num_of_channels++;
    return 0;
}

pgchannel get_gchannel(pbboard bboardh,void* data){
    bboard_channel tmp;
    tmp.data=malloc(get_type_size(data_info));
    if(tmp.data==NULL){
        fprintf(stderr,"Error allocating space -bboard.c\n");
        exit(1);
    }
    memcpy(tmp.data,data,get_type_size(data_info));
    /*printf("get_gchannel call with\n");*/
    pgchannel rt;
    rt= ret_data(get_node(bboardh->channels,&tmp));
    free (tmp.data);
    return rt;
}

pmessage get_gmessage(const pgchannel M,pmessage tmp){
    pmessage  rt;
    rt= ret_data(get_node(M->messages,tmp));
    return rt;
}

int remove_message_d(pbboard bboardh,void *M){
    pgchannel a = get_gchannel(bboardh,M);
    if(a==NULL){
        return 1;
    }
    else{
        /*puts("remove edge in d call");*/
        return remove_message(bboardh,a);
    }
}

int add_message_d(pbboard bboardh,void * M,char * msg,time_t fstmsgtm,size_t msgsize ,char type, char flag, int mid,int pid,int rcvd){
    pgchannel a = get_gchannel(bboardh,M);
    message tmp;
    tmp.msgsize=msgsize;
    tmp.type=type;
    tmp.mid=mid;
    tmp.pid=pid;
    tmp.fst_msg_tm=fstmsgtm;
    tmp.full=flag;
    if(a==NULL){
        return 1;
    }
    else{
        if(msginchannel(a,&tmp)!=1){
            a->out+=1;
            add_message(bboardh,a,msg,fstmsgtm,msgsize,type,flag,mid,pid,rcvd);
        }
        else{
            pmessage pmsg=get_gmessage(a,&tmp);
            if(type=='w')
                strcat(pmsg->msg,msg);
            else{
                int offset=pmsg->rcvd;
                memcpy(pmsg->msg+offset,msg,rcvd);
            }
            pmsg->rcvd+=rcvd;
            pmsg->full=flag;
        }
    }
    return 0;
}

int add_message(pbboard bboardh,const pgchannel M,char *msg,time_t fstmsgtm,size_t msgsize ,char type, char flag, int mid,int pid,int rcvd){
    /*puts("add edge call");*/
    insert(M->messages, cr_message(msg,fstmsgtm,msgsize,type,flag,mid,pid,rcvd));
    return 0;
}

int remove_message(pbboard bboardh,const pgchannel M){
    if(pop_back(M->messages,message_destr)==0){
        M->out-=1;
        return 0;
    }
    return 1;
}

//deletes channel with data
int delete_gchannel(pbboard bboardh, void *data){
    /*printf("delete channel %d call\n",*(int*)data);*/
    pgchannel tmp;
    bboard_channel tmp2;
    tmp=get_gchannel(bboardh,data);
    if(tmp==NULL){
         return 1;
    }
    if(tmp->in!=0 || tmp->out!=0){
        return 2;
    }
    tmp2.data=malloc(get_type_size(data_info));
    if(tmp2.data==NULL){
        fprintf(stderr,"Error allocating space -bboard.c.\n");
        exit(1);
    }
    memcpy(tmp2.data,data,get_type_size(data_info));
    delete(bboardh->channels,&tmp2,channeldestr);
    bboardh->num_of_channels--;
    free (tmp2.data);
    /*puts("deleted\n");*/
    return 0;
}

//prints bboard
int print(pbboard bboardh, void (*print_dt)(void *)){
    pnode tmp,tmp2; //tmp for channels tmp2 for messages
    tmp=get_list(bboardh->channels);
    while(tmp!=NULL){
        printf("vertex(");
        print_dt(((pgchannel) (ret_data(tmp)))->data);
        tmp2=get_list(((pgchannel)(ret_data(tmp)))->messages);
        printf(") = ");
        int c=0;
        while(tmp2!=NULL){
            if(c!=0)
                printf(",(");
            else{
                c=1;
                printf("(");
            }
            printf(",%s,%d,%c,%c)",((pmessage)(ret_data(tmp2)))->msg,((pmessage)(ret_data(tmp2)))->msgsize,((pmessage)(ret_data(tmp2)))->type,((pmessage)(ret_data(tmp2)))->full);
            tmp2=next_node(tmp2);
        }
        tmp=next_node(tmp);
        puts("");
    }
    return 0;
}

//ds bboard destroys bboard
void ds_bboard(pbboard bboardh){
    ds_list_plus_data(bboardh->channels,channeldestr);
    free(bboardh->message_info);
    free(bboardh->channel_info);
    free(bboardh);
}

char* get_ready_messages(pbboard bboardh, void *data){
    pgchannel a;
    char *msg,buff[524];
    a=get_gchannel(bboardh,data);
    pnode tmp2;
    int size=0;
    if(a==NULL){
        msg=malloc(strlen("Couldn't find channel\n")+1);
        strcpy(msg,"Couldn't find channel\n");
        return msg;
    }
    else{
        tmp2=get_list(a->messages);
        while(tmp2!=NULL){
            if(((pmessage)(ret_data(tmp2)))->full=='y'){
                if(((pmessage)(ret_data(tmp2)))->type=='w'){
                    size+=((pmessage)(ret_data(tmp2)))->msgsize+strlen("Message:\n\n");
                    /*printf("Message:\n");*/
                    /*printf("%s\n",((pmessage)(ret_data(tmp2)))->msg);*/
                }
            }
            tmp2=next_node(tmp2);
        }
        msg=malloc(size+1);
        *msg='\0';
        tmp2=get_list(a->messages);
        while(tmp2!=NULL){
            if(((pmessage)(ret_data(tmp2)))->full=='y'){
                if(((pmessage)(ret_data(tmp2)))->type=='w'){
                    message tmp;
                    tmp.mid=((pmessage)(ret_data(tmp2)))->mid;
                    tmp.pid=((pmessage)(ret_data(tmp2)))->pid;
                    tmp.msgsize=((pmessage)(ret_data(tmp2)))->msgsize;
                    tmp.type=((pmessage)(ret_data(tmp2)))->type;
                    tmp.fst_msg_tm=((pmessage)(ret_data(tmp2)))->fst_msg_tm;
                    sprintf(buff,"Message:\n%s\n",((pmessage)(ret_data(tmp2)))->msg);
                    strcat(msg,buff);
                    delete(a->messages,&tmp,message_destr);
                }
            }
            tmp2=next_node(tmp2);
        }
    }
    /*printf("final:%s\n",msg);*/
    return msg;
}

char* get_ready_files(pbboard bboardh, void *data, int *filessize){
    pgchannel a;
    char *msg;
    a=get_gchannel(bboardh,data);
    pnode tmp2;
    int size=0;
    int offset=0;
    if(a==NULL){
        msg=malloc(2);
        msg='\0';
        return msg;
    }
    else{
        tmp2=get_list(a->messages);
        while(tmp2!=NULL){
            if(((pmessage)(ret_data(tmp2)))->full=='y'){
                if(((pmessage)(ret_data(tmp2)))->type=='f'){
                    size+=((pmessage)(ret_data(tmp2)))->msgsize+10;
                    /*printf("Message:\n");*/
                    /*printf("%s\n",((pmessage)(ret_data(tmp2)))->msg);*/
                }
            }
            tmp2=next_node(tmp2);
        }
        msg=malloc(size+1);
        *msg='\0';
        tmp2=get_list(a->messages);
        *filessize=size;
        while(tmp2!=NULL){
            if(((pmessage)(ret_data(tmp2)))->full=='y'){
                if(((pmessage)(ret_data(tmp2)))->type=='f'){
                    message tmp;
                    tmp.mid=((pmessage)(ret_data(tmp2)))->mid;
                    tmp.pid=((pmessage)(ret_data(tmp2)))->pid;
                    tmp.msgsize=((pmessage)(ret_data(tmp2)))->msgsize;
                    tmp.type=((pmessage)(ret_data(tmp2)))->type;
                    tmp.fst_msg_tm=((pmessage)(ret_data(tmp2)))->fst_msg_tm;
                    sprintf(msg+offset,"%10d",tmp.msgsize);
                    memcpy(msg+offset+10,((pmessage)(ret_data(tmp2)))->msg,tmp.msgsize);
                    offset+=tmp.msgsize+10;
                    delete(a->messages,&tmp,message_destr);
                }
            }
            tmp2=next_node(tmp2);
        }
    }
    /*printf("final:%s\n",msg);*/
    return msg;
}

char* get_channel_names(pbboard bboardh,void (*print_dt)(void*,char*)){
    pnode tmp;
    char buff[128],*tmpmsg;
    int length=0;
    tmp=get_list(bboardh->channels);
    while(tmp!=NULL){
        print_dt(((pgchannel) (ret_data(tmp)))->data,buff);
        length+=strlen(buff);
        tmp=next_node(tmp);
    }
    if((tmpmsg=malloc(length+1))==NULL){
        perror("Malloc failed in get_channel_names.\n");
        return NULL;
    }
    *tmpmsg='\0';
    tmp=get_list(bboardh->channels);
    while(tmp!=NULL){
        print_dt(((pgchannel) (ret_data(tmp)))->data,buff);
        strcat(tmpmsg,buff);
        tmp=next_node(tmp);
    }
    /*printf("%s %d %d",tmpmsg,(int)strlen(tmpmsg)+1,(int)length+1);*/
    return tmpmsg;
}
