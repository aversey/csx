#ifndef CSX_INCLUDED
#define CSX_INCLUDED


typedef void *(csx_list_fn)(void *head, ...);
void *csx_list(void *head, ...);

typedef void *(csx_dot_fn)(void *a, void *b, void *c, ...);
void *csx_dot(void *a, void *b, void *c, ...);

typedef int *(csx_int_fn)(int num);
int *csx_int(int num);

void *csx_run(void *expression);

typedef void *(*csx_base_data)(void *arg);
csx_base_data *csx_base(csx_base_data base);

char *csx_name(const char *name);


#endif
