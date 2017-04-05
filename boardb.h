#ifndef _bboard_
#define _bboard_
#include "stdio.h"
#include "element.h"
#include "list.h"
#define CHAN_TTL_S 128


typedef struct bboard_info * pbboard;
typedef struct bboard_channel * pgchannel;
typedef struct message * pmessage;

//CREATION
pbboard cr_bboard (pel_info pinfo);
pgchannel cr_gchannel (pbboard bboardh,const void *data);           //creates a channel to use in bboard
pmessage cr_message(char* msg,
    time_t fstmsgtm,
    size_t msgsize,
    char type,
    char flag,
    int mid,
    int pid,
    int rcvd);
//DESTROY
void ds_bboard (pbboard gtodestroy);                                //destroys bboard and frees all data
void ds_gchannel (pgchannel gntodestroy);                           //destroys a channel
void ds_message (pmessage atodestroy);                              //destroys a message

//ACCESS
int in_bboard(pbboard bboardh,void *data);                          //returns 1 if data exists in bboard else 0
pgchannel get_gchannel(pbboard bboardh ,void* data);                //returns pgchannel with data
pmessage get_gmessage(const pgchannel M,pmessage tmp);
int print(pbboard bboardh, void (*print_dt)(void *));               //prints bboard needs a function to print data
char* get_channel_names(pbboard bboardh,void (*print_dt)(void*,char*));   //returns pointer to a string with all channel names and ids
int msginchannel(const pgchannel M,pmessage a);
int msginboard(pbboard bboardh,
        void *data,
        time_t fstmsgtm,
        size_t msgsize,
        char type,
        int mid,
        int pid);

//MODIFY
char* get_ready_messages(pbboard bboardh, void *data);              //returns pointer to a string with all txt type messages and deletes them ,string needs free after
char* get_ready_files(pbboard bboardh, void *data, int *filessize); //returns pointer to a string with all files data and deletes file data from board needs free after
int ginsert (pbboard bboardh,void *data);                           //inserts data in bboard ,if there is already the same channel 1 will be returned
int add_message(pbboard bboardh,const pgchannel M,char *msg,time_t fstmsgtm,
                size_t msgsize ,char type, char flag, int mid,int pid,int rcvd);
int add_message_d(pbboard bboardh,void * M,char * msg,time_t fstmsgtm,size_t msgsize ,
        char type, char flag, int mid,int pid,int rcvd);                     //add string in msg
int remove_message_d(pbboard bboardh,void *M);                      //removes edge between channels with data M, Y success 0 failure 1
int remove_message (pbboard bboardh, const pgchannel M);            //removes an edge between channels M Y uses the same return value as the above
int delete_gchannel(pbboard bboardh, void *data);                                //deletes a channel returns 1 if channel doesn't exist
#endif
