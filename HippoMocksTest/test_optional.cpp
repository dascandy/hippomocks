#include "hippomocks.h"
#include "gtest/gtest.h"

class II { 
public:
	virtual ~II() {}
	virtual void f() {}
	virtual void g() = 0;
};

TEST (TestOptional, checkBaseCaseOptionalOk)
{
	MockRepository mocks;
	II *iamock = mocks.Mock<II>();
	mocks.ExpectCall(iamock, II::f);
	mocks.OnCall(iamock, II::g);
	iamock->g();
	iamock->f();
}

TEST (TestOptional, checkBaseCaseOptionalNotRequired)
{
	MockRepository mocks;
	II *iamock = mocks.Mock<II>();
	mocks.ExpectCall(iamock, II::f);
	mocks.OnCall(iamock, II::g);
	iamock->f();
}

TEST (TestOptional, checkBaseCaseOptionalTwiceIsOK)
{
	MockRepository mocks;
	II *iamock = mocks.Mock<II>();
	mocks.ExpectCall(iamock, II::f);
	mocks.OnCall(iamock, II::g);
	iamock->g();
	iamock->f();
	iamock->g();
}

