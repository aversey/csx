#include <csx.h>


v def_rat_sum(v construct, v numer, v denom)
{
    v a = atom("a");
    v b = atom("b");
    return el(define, l(atom("rat_sum"), a, b),
        l(construct, l(sum, l(mul, l(numer, a), l(denom, b)),
                            l(mul, l(numer, b), l(denom, a))),
                     l(mul, l(denom, a), l(denom, b)))
    );
}

v def_rat_sum(v construct, v numer, v denom)
{
    v a = atom("a");
    v b = atom("b");
    return el(define, l(atom("rat_sub"), a, b),
        l(construct, l(sub, l(mul, l(numer, a), l(denom, b)),
                            l(mul, l(numer, b), l(denom, a))),
                     l(mul, l(denom, a), l(denom, b)))
    );
}

v def_rat_equ(v construct, v numer, v denom)
{
    v a = atom("a");
    v b = atom("b");
    return el(define, l(atom("rat_equ"), a, b),
        l(equ, l(mul, l(numer, a), l(denom, b)),
               l(mul, l(numer, b), l(denom, a)))
    );
}

void printres(v a, v b, v absum, v absub, int *are_equ, v numer, v denom)
{
    int an   = *(int *)el(numer, a);
    int ad   = *(int *)el(denom, a);
    int bn   = *(int *)el(numer, b);
    int bd   = *(int *)el(denom, b);
    int sumn = *(int *)el(numer, absum);
    int sumd = *(int *)el(denom, absum);
    int subn = *(int *)el(numer, absub);
    int subd = *(int *)el(denom, absub);
    printf("%d/%d + %d/%d = %d/%d\n", an, ad, bn, bd, sumn, sumd);
    printf("%d/%d - %d/%d = %d/%d\n", an, ad, bn, bd, subn, subd);
    if (*are_equ) {
        puts("And they are equal.\n");
    } else {
        puts("And they are not equal.\n");
    }
}

int main()
{
    v rat       = l(define, atom("rat"), cons);
    v rat_numer = l(define, atom("rat_numer"), car);
    v rat_denom = l(define, atom("rat_denom"), cdr);
    v rat_sum   = def_rat_sum(rat, rat_numer, rat_denom);
    v rat_sub   = def_rat_sub(rat, rat_numer, rat_denom);
    v rat_equ   = def_rat_equ(rat, rat_numer, rat_denom);
    v number_a  = el(rat, n(19), n(99));
    v number_b  = el(rat, n(7), n(3));
    v absum     = el(rat_sum, number_a, number_b);
    v absub     = el(rat_sub, number_a, number_b);
    v are_equ   = el(rat_equ, number_a, number_b);
    printres(number_a, number_b, sum, sub, are_equ, numer, denom);
    return 0;
}
