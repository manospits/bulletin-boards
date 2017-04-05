#define HDR_SIZE 62
#define HEADER_SIZE_INFO 10
#define MSG_SIZE 512

void cr_header(char * a,
        int fstmsgtm,
        char type,
        int id,
        int pid,
        int mid,
        int pt,
        char f,
        int pt_size,
        int msg_size);

int get_header_size(char * header);

int isnumber(char *a); //checks if string a is a number

void rmn(char * a); //replaces character '\n' with '\0'

int count_spaces(char* a); //counts spaces in string a
