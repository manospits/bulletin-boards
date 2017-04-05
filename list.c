#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "element.h"

typedef struct node {
    pel data;
    struct node* next;
} node;

typedef struct head{
    int size;                               //list size
    pnode front;                            //first node
    pnode end;                              //last node
    pel_info data_info;                     //data info
} head;

void NO_DESTR(void* a){
    return;
}

int get_size(phead listh){
    return listh->size;
}

phead cr_list(pel_info data_info){
    phead tmphead;
    tmphead=malloc(sizeof(struct head));
    tmphead->size=0;
    tmphead->front=NULL;
    tmphead->end=NULL;
    tmphead->data_info=data_info;
    return tmphead;
}

pnode cr_node(phead listh,void *data){
    node* tmpnode;
    tmpnode=malloc(sizeof(struct node));
    tmpnode->data=create_elem(listh->data_info,data);
    tmpnode->next=NULL;
    return tmpnode;
}

void ds_node(phead listh,pnode ntodestroy){
    type_destr(listh->data_info,ntodestroy->data);
    free(ntodestroy);
}

void ds_list(phead ltodestroy){
    if(ltodestroy==NULL){
        return;
    }
    pnode todel;
    pnode next=ltodestroy->front;
    while(next!=NULL){
        todel=next;
        next=next->next;
        ds_node(ltodestroy,todel);
    }
    free (ltodestroy);
}

void ds_list_plus_data(phead ltodestroy,void (*destr)(void *)){
    if(ltodestroy==NULL){
        return;
    }
    pnode todel;
    pnode next=ltodestroy->front;
    while(next!=NULL){
        todel=next;
        next=next->next;
        destr(get_ptr(todel->data));
        ds_node(ltodestroy,todel);
    }
    free (ltodestroy);
}

void insert(phead listh,void *data){
    pnode nptr=cr_node(listh,data);
    if(listh->size==0){
        listh->front=nptr;
        listh->end=nptr;
        listh->size++;
    }
    else{
        nptr->next=listh->front;
        listh->front=nptr;
        listh->size++;
    }
}

void insert_back(phead listh,void *data){
    pnode nptr=cr_node(listh,data);
    if(listh->size==0){
        listh->front=nptr;
        listh->end=nptr;
        listh->size++;
    }
    else{
        listh->end->next=nptr;
        listh->end=nptr;
        listh->size++;
    }
}

void delete(phead listh,void *data,void (*destr)(void *)){
    /*puts("delete call");*/
    pel pdata=create_elem(listh->data_info,data);
    if(listh->size==1 && (type_cmp(listh->data_info,pdata,listh->front->data))==0){
        destr(get_ptr(listh->front->data));
        ds_node(listh,listh->front);
        listh->front=NULL;
        listh->end=NULL;
        listh->size=0;
    }
    else if (listh->size > 1){
        pnode prev=listh->front;
        pnode todel=listh->front;
        pnode next;
        while(todel!=NULL){
            if(type_cmp(listh->data_info,todel->data,pdata)==0){
                if(listh->size==1){
                    listh->end=NULL;
                    listh->front=NULL;
                }
                else if(listh->front==todel){
                    listh->front=todel->next;
                }
                else if(listh->end==todel){
                    listh->end=prev;
                    prev->next=NULL;
                }
                else{
                    prev->next=todel->next;
                }
                next=todel->next;
                destr(get_ptr(todel->data));
                ds_node(listh,todel);
                todel=next;
                listh->size--;
            }
            else{
                prev=todel;
                todel=todel->next;
            }
        }
    }
    type_destr(listh->data_info,pdata);
}

int pop_back(phead listh,void(*destr)(void*)){
    if(listh->size < 1){
        return 1;
    }
    if(listh->size==1){
        destr(get_ptr(listh->front->data));
        ds_node(listh,listh->front);
        listh->front=NULL;
        listh->end=NULL;
        listh->size=0;
    }
    else{
        pnode todel=listh->front;
        pnode prev=listh->front;
        while(todel!=listh->end){
            prev=todel;
            todel=todel->next;
        }
        listh->end=prev;
        prev->next=NULL;
        destr(get_ptr(todel->data));
        ds_node(listh,todel);
        listh->size--;
    }
    return 0;
}

void* ret_data(pnode nd){
    if(nd==NULL){
        return NULL;
    }
    else{
        return get_ptr(nd->data);
    }
}

pnode get_list(const phead listh){
    if(listh==NULL){
        return NULL;
    }
    else{
        return listh->front;
    }
}

pnode next_node(const pnode nd){
    if(nd==NULL){
        return NULL;
    }
    else{
        return nd->next;
    }
}

int intcmp(const void* a, const void* b){
    if (*(int*)a==*(int*)b) return 0;
    else if(*(int*)a < *(int*)b) return -1;
    else return 1;
}

int in(phead listh,void *data){
    pnode tmp=listh->front;
    pel a=create_elem(listh->data_info,data);
    while(tmp!=NULL){
        if(type_cmp(listh->data_info,a,tmp->data)==0){
            type_destr(listh->data_info,a);
            return 1;
        }
        tmp=tmp->next;
    }
    type_destr(listh->data_info,a);
    return 0;
}

pnode get_node(phead listh, void *data){
    pnode tmp=listh->front;
    pel a=create_elem(listh->data_info,data);
    while(tmp!=NULL){
        if(type_cmp(listh->data_info,a,tmp->data)==0){
            type_destr(listh->data_info,a);
            return tmp;
        }
        tmp=tmp->next;
    }
    type_destr(listh->data_info,a);
    return NULL;
}

