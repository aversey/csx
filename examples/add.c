#include <csx.h>
#include <stdio.h>


static csx_list_fn *l;
static csx_int_fn *i;


int main()
{
    l = csx_list;
    i = csx_int;
    char *plus = csx_name("+");
    printf("%d\n", *(int *)csx_run(l(plus, i(37), i(73), 0)));
    csx_free();
    return 0;
}
