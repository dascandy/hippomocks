#include "hippomocks.h"
#include "gtest/gtest.h"
#include "target_cfuncs.h"

// If it's not supported, then don't test it.
#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT

int void_test;

TEST (TestCfuncs, checkFunctionReplacedAndChecked)
{
	EXPECT_EQ(ret_2(), 2);
	MockRepository mocks;
	mocks.ExpectCallFunc(ret_2).Return(7);
	EXPECT_EQ(ret_2(), 7);
}

TEST (TestCfuncs, checkFunctionReturnedToOriginal)
{
	{
		EXPECT_EQ(ret_1(), 1);
		MockRepository mocks;
		mocks.ExpectCallFunc(ret_1).Return(5);
		EXPECT_EQ(ret_1(), 5);
	}
	EXPECT_EQ(ret_1(), 1);
}

TEST (TestCfuncs, checkOrderFunctionReturnedToOriginal)
{
	{
        EXPECT_EQ(ret_1(), 1);
        EXPECT_EQ(ret_2(), 2);
		MockRepository mocks;
		mocks.ExpectCallFunc(ret_2).Return(7);
		mocks.ExpectCallFunc(ret_1).Return(5);
		EXPECT_EQ(ret_2(), 7);
		EXPECT_EQ(ret_1(), 5);
	}
    EXPECT_EQ(ret_1(), 1);
    EXPECT_EQ(ret_2(), 2);

    /* In reversed order */
    {
        MockRepository mocks;
        mocks.ExpectCallFunc(ret_1).Return(5);
        mocks.ExpectCallFunc(ret_2).Return(7);
        EXPECT_EQ(ret_1(), 5);
        EXPECT_EQ(ret_2(), 7);
    }
    EXPECT_EQ(ret_1(), 1);
    EXPECT_EQ(ret_2(), 2);;
}

#ifdef _WIN32
#include <windows.h>
TEST (TestCfuncs, checkCanMockGetSystemTime) {
	MockRepository mocks;
	SYSTEMTIME outtime;
	outtime.wDay = 1;
	SYSTEMTIME systime;
	systime.wDay = 0;
	mocks.ExpectCallFunc(GetSystemTime).With(Out(outtime));
	GetSystemTime(&systime);
	EXPECT_EQ(systime.wDay, 1);
}
#endif

#endif

