#include "hippomocks.h"
#include "Framework.h"

// If it's not supported, then don't test it.
#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
int a()
{
  return 1;
}

TEST (checkFunctionReplacedAndChecked)
{
	EQUALS(a(), 1);
	MockRepository mocks;
	mocks.ExpectCallFunc(a).Return(2);
	EQUALS(a(), 2);
}

TEST (checkFunctionReturnedToOriginal)
{
	{
		EQUALS(a(), 1);
		MockRepository mocks;
		mocks.ExpectCallFunc(a).Return(2);
		EQUALS(a(), 2);
	}
	EQUALS(a(), 1);
}
#endif

