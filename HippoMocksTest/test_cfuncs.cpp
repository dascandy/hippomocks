#include "hippomocks.h"
#include "Framework.h"
#include "target_cfuncs.h"

// If it's not supported, then don't test it.
#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT

TEST (checkFunctionReplacedAndChecked)
{
	EQUALS(ret_2(), 2);
	MockRepository mocks;
	mocks.ExpectCallFunc(ret_2).Return(7);
	EQUALS(ret_2(), 7);
}

TEST (checkFunctionReturnedToOriginal)
{
	{
		EQUALS(ret_1(), 1);
		MockRepository mocks;
		mocks.ExpectCallFunc(ret_1).Return(5);
		EQUALS(ret_1(), 5);
	}
	EQUALS(ret_1(), 1);
}

TEST (checkOrderFunctionReturnedToOriginal)
{
	{
        EQUALS(ret_1(), 1);
        EQUALS(ret_2(), 2);
		MockRepository mocks;
		mocks.ExpectCallFunc(ret_2).Return(7);
		mocks.ExpectCallFunc(ret_1).Return(5);
		EQUALS(ret_2(), 7);
		EQUALS(ret_1(), 5);
	}
    EQUALS(ret_1(), 1);
    EQUALS(ret_2(), 2);

    /* In reversed order */
    {
        MockRepository mocks;
        mocks.ExpectCallFunc(ret_1).Return(5);
        mocks.ExpectCallFunc(ret_2).Return(7);
        EQUALS(ret_1(), 5);
        EQUALS(ret_2(), 7);
    }
    EQUALS(ret_1(), 1);
    EQUALS(ret_2(), 2);;
}

#ifdef _WIN32
#include <windows.h>
TEST (checkCanMockGetSystemTime) {
	MockRepository mocks;
	SYSTEMTIME outtime;
	outtime.wDay = 1;
	SYSTEMTIME systime;
	systime.wDay = 0;
	mocks.ExpectCallFunc(GetSystemTime).With(Out(outtime));
	GetSystemTime(&systime);
	EQUALS(systime.wDay, 1);
}
#endif

#endif

