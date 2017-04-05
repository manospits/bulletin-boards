#ifndef _element_
#define _element_
#include <stdlib.h>

typedef struct element_info * pel_info;
typedef struct element* pel;

//creating info about the type you want to use,
//must use free(name_of_the_element_info) to clean
pel_info create_type(size_t size,                       //size of the data
                 int (*cmp)(const void*,const void *)); //comparator to use
//create a data element
pel create_elem(pel_info,void *);
//compares to elements
int type_cmp(pel_info,pel,pel);
//returns comparator
int (*get_cmp(pel_info a))(const void *,const void *);
//returns data size
size_t get_type_size(pel_info);
//destroys an element
void type_destr(pel_info,pel);
//get ptr to the data
void * get_ptr(pel);
//set the data
void set_data(pel,void *);
#endif
