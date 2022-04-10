#include "gtest/gtest.h"
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

TEST (TestDontCare, checkDontcareIsIgnored)
{
	MockRepository mocks;
	IY *iamock = mocks.Mock<IY>();
	mocks.OnCall(iamock, IY::test).Return(false);
	mocks.OnCall(iamock, IY::test).With(42, _).Return(true);
	IZ iz;
	EXPECT_EQ(true, iamock->test (42, iz));
	EXPECT_EQ(false, iamock->test (40, iz));
}

