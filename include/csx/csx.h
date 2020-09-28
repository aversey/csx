#ifndef CSX_INCLUDED
#define CSX_INCLUDED


struct csx_pair {
    void *car;
    void *cdr;
};

typedef struct csx_pair *(csx_type_function_list)(void *head, ...);
struct csx_pair *csx_list(void *head, ...);

typedef char *(csx_type_function_string)(const char *string);
char *csx_string(const char *string);

typedef char *(csx_type_function_atom)(const char *name);
char *csx_atom(const char *name);

typedef void *(csx_type_function_evaluate)(void *expression);
void *csx_evaluate(void *expression);

void csx_free();


#endif
