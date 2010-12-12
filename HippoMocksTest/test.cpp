#include "hippomocks.h"
#include "yaffut.h"

class IA { 
public:
	virtual ~IA() {}
	virtual void f() {}
	virtual void g() = 0;
};

FUNC (checkBaseCase)
{
	MockRepository mocks;
	IA *iamock = mocks.Mock<IA>();
	mocks.ExpectCall(iamock, IA::f);
	mocks.ExpectCall(iamock, IA::g);
	iamock->f();
	iamock->g();
}

FUNC (checkMultiCall)
{
	MockRepository mocks;
	IA *iamock = mocks.Mock<IA>();
	mocks.ExpectCall(iamock, IA::f);
	mocks.ExpectCall(iamock, IA::g);
	mocks.ExpectCall(iamock, IA::f);
	iamock->f();
	iamock->g();
	iamock->f();
}

FUNC (checkMultiCallNotCalled)
{
	bool exceptionCaught = false;
	try {
		MockRepository mocks;
		IA *iamock = mocks.Mock<IA>();
		mocks.ExpectCall(iamock, IA::f);
		mocks.ExpectCall(iamock, IA::g);
		mocks.ExpectCall(iamock, IA::f);
		iamock->f();
		iamock->g();
	}
	catch (HippoMocks::CallMissingException &) 
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
}

FUNC (checkMultiCallWrongOrder)
{
	MockRepository mocks;
	IA *iamock = mocks.Mock<IA>();
	mocks.ExpectCall(iamock, IA::f);
	mocks.ExpectCall(iamock, IA::g);
	mocks.ExpectCall(iamock, IA::f);
	iamock->f();
	bool exceptionCaught = false;
	try {
		iamock->f();
	}
	catch (HippoMocks::ExpectationException &) 
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
	mocks.reset();
}

FUNC (checkExpectationsNotCompleted)
{
	bool exceptionCaught = false;
	try {
		MockRepository mocks;
		IA *iamock = mocks.Mock<IA>();
		mocks.ExpectCall(iamock, IA::f);
		mocks.ExpectCall(iamock, IA::g);
		iamock->f();
	}
	catch (HippoMocks::CallMissingException &) 
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
}

FUNC (checkOvercompleteExpectations)
{
	bool exceptionCaught = false;
	try {
		MockRepository mocks;
		IA *iamock = mocks.Mock<IA>();
		mocks.ExpectCall(iamock, IA::f);
		mocks.ExpectCall(iamock, IA::g);
		iamock->f();
		iamock->g();
		iamock->f();
	}
	catch (HippoMocks::ExpectationException &) 
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
}

FUNC (checkExpectationsAreInOrder)
{
	bool exceptionCaught = false;
	try {
		MockRepository mocks;
		IA *iamock = mocks.Mock<IA>();
		mocks.ExpectCall(iamock, IA::f);
		mocks.ExpectCall(iamock, IA::g);
		iamock->g();
	}
	catch (HippoMocks::ExpectationException &) 
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
}

