#include "hippomocks.h"

void (base_mock::*base_mock::base_func(int index))()
{
	curFuncNo = index;
	funcs[index] = next_func;
	return next_func;
}
