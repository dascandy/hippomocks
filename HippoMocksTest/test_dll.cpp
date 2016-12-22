#include "hippomocks.h"
#include "Framework.h"
#include "DllApi.h"

#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT

TEST(dllFuncCalled)
{
	MockRepository mocks;
	mocks.ExpectCallFunc(DllSum1).Return(-1);
	mocks.ExpectCallFunc(DllSum2).Return(-1);
	mocks.ExpectCallFunc(DllSum3).Return(-1);
	mocks.ExpectCallFunc(DllSum4).Return(-1);
	EQUALS(DllSum1(), -1);
	EQUALS(DllSum2(), -1);
	EQUALS(DllSum3(), -1);
	EQUALS(DllSum4(), -1);
}

#endif