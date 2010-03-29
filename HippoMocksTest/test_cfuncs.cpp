#include "hippomocks.h"
#include "yaffut.h"

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
	EQUAL(a(), 1);
}

