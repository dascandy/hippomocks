#include "hippomocks.h"
#include "gtest/gtest.h"

class IM { 
public:
	virtual ~IM() {}
	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void a() = 0;
	virtual void b() = 0;
};

TEST (TestTransaction, checkTransactionStyleWorks)
{
	MockRepository mocks;
	IM *iamock = mocks.Mock<IM>();
	mocks.autoExpect = false;
	Call &beginCall = mocks.ExpectCall(iamock, IM::begin);
	Call &aCall = mocks.ExpectCall(iamock, IM::a).After(beginCall);
	Call &bCall = mocks.ExpectCall(iamock, IM::b).After(beginCall);
	mocks.ExpectCall(iamock, IM::end).After(aCall).After(bCall);
	iamock->begin();
	iamock->b();
	iamock->a();
	iamock->end();
}

TEST (TestTransaction, checkTransactionStyleFailIfOneSkipped)
{
	MockRepository mocks;
	IM *iamock = mocks.Mock<IM>();
	mocks.autoExpect = false;
	Call &beginCall = mocks.ExpectCall(iamock, IM::begin);
	Call &aCall = mocks.ExpectCall(iamock, IM::a).After(beginCall);
	Call &bCall = mocks.ExpectCall(iamock, IM::b).After(beginCall);
	mocks.ExpectCall(iamock, IM::end).After(aCall).After(bCall);
	iamock->begin();
	iamock->b();
	bool exceptionCaught = false;
	try {
		iamock->end();
	}
	catch (HippoMocks::ExpectationException &) 
	{
		exceptionCaught = true;
	}
	EXPECT_TRUE(exceptionCaught);
	mocks.reset();
}

