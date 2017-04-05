#include "element.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct element_info {
    size_t size;
    int flag;
    int (*cmp)(const void*,const void *);
}element_info;

typedef struct element{
    void * elem;
}element;

pel_info create_type(size_t size,
                 int (*cmp)(const void*,const void *)){
    pel_info tmp=malloc(sizeof(struct element_info));
    if(tmp==NULL){
        fprintf(stderr,"Error allocating space -element.c.\n");
        exit(1);
    }
    tmp->size=size;
    tmp->cmp=cmp;
    return tmp;
}

pel create_elem(pel_info pinfo,void * a){
    pel tmp=malloc(sizeof(struct element));
    if(tmp==NULL){
        fprintf(stderr,"Error allocating space -element.c.\n");
        exit(1);
    }
    tmp->elem=a;
    return tmp;
}

int type_cmp(pel_info pinfo,pel a,pel b){
    return pinfo->cmp(a->elem,b->elem);
}

void type_destr(pel_info pinfo, pel a){
    free(a);
}

size_t get_type_size(pel_info a){
    return a->size;
}

int (*get_cmp(pel_info a))(const void *,const void *)
{
    return a->cmp;
}
void *get_ptr(pel a){
   return a->elem;
}
