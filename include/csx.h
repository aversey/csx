#ifndef CSX_INCLUDED
#define CSX_INCLUDED


typedef void *(csx_list_fn)(void *head, ...);
void *csx_list(void *head, ...);

typedef int *(csx_num_fn)(int num);
int *csx_num(int num);

void *csx_eval(void *expression);

typedef void *(*csx_base_data)(void *arg);
void *csx_base(csx_base_data base);

char *csx_name(const char *name);


#endif
