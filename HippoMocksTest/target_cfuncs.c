#include "target_cfuncs.h"

#ifdef _MSC_VER
#pragma optimize( "fsa", on)
#endif

int ret_1(void)
{
    return 1;
}

int ret_2(void)
{
    return 2;
}

void ret_3(void)
{
    void_test = 3;
}

void ret_4(void)
{
    void_test = 4;
}
