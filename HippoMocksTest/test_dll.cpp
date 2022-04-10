#include "hippomocks.h"
#include "gtest/gtest.h"
#include "DllApi.h"

#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT

TEST (TestDll, dllFuncCalled)
{
	MockRepository mocks;
	mocks.ExpectCallFunc(DllSum1).Return(-1);
	mocks.ExpectCallFunc(DllSum2).Return(-1);
	mocks.ExpectCallFunc(DllSum3).Return(-1);
	mocks.ExpectCallFunc(DllSum4).Return(-1);
	EXPECT_EQ(DllSum1(), -1);
	EXPECT_EQ(DllSum2(), -1);
	EXPECT_EQ(DllSum3(), -1);
	EXPECT_EQ(DllSum4(), -1);
}

#endif