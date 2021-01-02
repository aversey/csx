#include <csx.h>
#include <stdio.h>


static csx_list_fn *l;
static csx_num_fn  *n;

char *define;
char *fn;
char *pair;
char *head;
char *tail;
char *not;
char *csx_if;
char *inc;
char *sum;
char *diff;

void process()
{
    char *fib = csx_name("fib");
    char *num = csx_name("num");
    csx_eval(l(define, fib, l(fn, l(num, 0),
        l(csx_if, l(inc, num, n(2), 0), n(1),
            l(sum,
                l(fib, l(diff, num, n(1), 0), 0),
                l(fib, l(diff, num, n(2), 0), 0),
            0),
        0),
    0), 0));
    printf("fib 6 = %d\n", *(int *)csx_eval(l(fib, n(6), 0)));
}

int main()
{
    l = csx_list;
    n = csx_num;
    define = csx_name("define");
    fn = csx_name("fn");
    pair = csx_name("pair");
    head = csx_name("head");
    tail = csx_name("tail");
    not = csx_name("not");
    csx_if = csx_name("if");
    inc = csx_name("inc");
    sum = csx_name("sum");
    diff = csx_name("diff");
    process();
    return 0;
}
