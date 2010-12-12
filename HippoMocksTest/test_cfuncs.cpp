#include "hippomocks.h"
#include "yaffut.h"

// If you add a new target for Cfunction mocking, add it here too
#if (defined(_MSC_VER) && defined(_M_IX86)) || (defined(__GNUC__) && defined(__i386__))

int a()
{
  return 1;
}

FUNC (checkFunctionReplacedAndChecked)
{
	EQUAL(a(), 1);
	MockRepository mocks;
	mocks.ExpectCallFunc(a).Return(2);
	EQUAL(a(), 2);
}

FUNC (checkFunctionReturnedToOriginal)
{
	{
		EQUAL(a(), 1);
		MockRepository mocks;
		mocks.ExpectCallFunc(a).Return(2);
		EQUAL(a(), 2);
	}
	EQUAL(a(), 1);
}

#endif

