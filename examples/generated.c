#include <csx.h>
int main()
{
csx_eval(csx_list(csx_name("do"),csx_list(csx_name("define"),csx_name("a"),csx_num(10),0),csx_list(csx_name("define"),csx_name("b"),csx_list(csx_name("sum"),csx_name("a"),csx_num(3),0),0),csx_list(csx_name("outname"),csx_list(csx_name("quote"),csx_name("sum-of-10-and-3-is"),0),0),csx_list(csx_name("outnum"),csx_name("b"),0),0));
return 0;
}
