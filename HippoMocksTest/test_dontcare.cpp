#include "yaffut.h"
#include "hippomocks.h"

class IZ { 
public:
	virtual ~IZ() {}
};

class IY {
public:
	virtual bool test(int a, IZ &b);
};

bool operator==(const IZ &, const IZ &)
{
	return false;
}

FUNC (checkDontcareIsIgnored)
{
	MockRepository mocks;
	IY *iamock = mocks.Mock<IY>();
	mocks.OnCall(iamock, IY::test).With(42, _).Return(true);
	mocks.OnCall(iamock, IY::test).Return(false);
	IZ iz;
	EQUAL(true, iamock->test(42, iz));
	EQUAL(false, iamock->test(40, iz));
}

