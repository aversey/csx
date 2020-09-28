#include <csx/csx.h>
#include <stdio.h>


static csx_type_function_list     *l;
static csx_type_function_string   *s;
static csx_type_function_atom     *a;
static csx_type_function_evaluate *e;

static char *append;
static char *define;
static char *cdr;
static char *ifx;
static char *not;

void printfibs(char *strfib)
{
    const char *input = "oooooooooo" + 9;
    int i;
    for (i = 0; i != 10; ++i)
        puts(e(l(strfib, s(input - i), 0)));
}

void process()
{
    char *strfib   = a("strfib");
    char *n        = a("n");
    e(l(define, l(strfib, n, 0),
        l(ifx, l(not, n, 0), s(""),
            l(ifx, l(not, l(cdr, n, 0), 0), s("o"),
                l(append, l(strfib, l(cdr, n, 0), 0),
                          l(strfib, l(cdr, l(cdr, n, 0), 0), 0), 0),
            0),
        0),
    0));
    printfibs(strfib);
}

int main()
{
    l = csx_list;
    s = csx_string;
    a = csx_atom;
    e = csx_evaluate;
    append = a("append");
    define = a("define");
    cdr    = a("cdr");
    ifx    = a("ifx");
    not    = a("not");
    process();
    csx_free();
    return 0;
}
