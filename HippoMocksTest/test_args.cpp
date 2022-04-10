#include "hippomocks.h"
#include "gtest/gtest.h"

class IB {
public:
	virtual ~IB() {}
	virtual void f(int);
	virtual void g(int) = 0;
};

TEST (TestArgs, checkArgumentsAccepted)
{
	MockRepository mocks;
	IB *iamock = mocks.Mock<IB>();
	mocks.ExpectCall(iamock, IB::f).With(1);
	mocks.ExpectCall(iamock, IB::g).With(2);
	iamock->f(1);
	iamock->g(2);
}

TEST (TestArgs, checkArgumentsChecked)
{
	MockRepository mocks;
	IB *iamock = mocks.Mock<IB>();
	mocks.ExpectCall(iamock, IB::f).With(1);
	mocks.ExpectCall(iamock, IB::g).With(1);

	EXPECT_THROW(iamock->f(2), HippoMocks::ExpectationException);
	mocks.reset();
}

