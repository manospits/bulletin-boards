#ifndef _LIST_
#define _LIST_
#include "element.h"

typedef struct node * pnode;
typedef struct head * phead;

//CREATION
phead cr_list(pel_info data_info);
pnode cr_node(phead listh,void *data);
void NO_DESTR(void *);

//DESTROY
void ds_list(phead ltodestroy);               //destroys list but doesn't frees allocate datatypes
void ds_list_plus_data(phead ltodestroy,void (*destr)(void *)); //destroys list and alllocated datatypes use NO_DESTR if you dont want
                                                                //to free data
void ds_node(phead listh,pnode ntodestroy);  //destroys a node

//MODIFY
void insert(phead listh,void *data);          //inserts node in front
void insert_back(phead listh,void *data);     //inserts node in back
void delete(phead listh,void *data,void (*destr)(void*));  //deletes a node (CALL WITH NO_DESTR if you dont want to free the datatype)
int pop_back(phead listh,void (*destr)(void*));
//ACCESS
int in(phead listh,void *data);               //checks if there is an element in the list
int get_size(phead listh);                    //get number of elements
void* ret_data(const pnode nd);               //returns a ptr to data
pnode get_list(const phead listh);            //returns ptr to the first node in list
pnode next_node(const pnode nd);              //retuns next node of pnode
pnode get_node(phead listh,void* data);       //returns node with data

//COMPARATORS
//comparators must return an integer, zero if a,b are equal, less than zero
//if a is less than b and greater than zero if a is greater than b
int intcmp(const void* a,const void* b);

#endif
