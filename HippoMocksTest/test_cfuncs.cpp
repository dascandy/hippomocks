#include "hippomocks.h"
#include "Framework.h"

// If you add a new target for Cfunction mocking, add it here too
#if (defined(_MSC_VER) && defined(_M_IX86)) || (defined(__GNUC__) && defined(__i386__))

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

