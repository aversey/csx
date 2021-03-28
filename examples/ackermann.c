#include <csxbind.h>
#include <stdio.h>


int main()
{
    csxbind_init();
    char *m = N("m");
    char *n = N("n");
    char *ackermann = N("ackermann");
    printf("%d\n", *(int *)R(L(_do,
      L(set, ackermann, L(fn, L(m, n, 0),
        L(_if,
          L(same, m, I(0), 0), L(_plus, n, I(1), 0),
          L(same, n, I(0), 0), L(ackermann, L(_plus, m, I(-1), 0), I(1), 0),
          L(ackermann,
            L(_plus, m, I(-1), 0),
            L(ackermann, m, L(_plus, n, I(-1), 0), 0),
          0),
        0),
      0), 0),
      L(ackermann, I(3), I(3), 0),
    0)));
    csxbind_free();
    return 0;
}
