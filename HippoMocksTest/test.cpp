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
	IA *iamock = mocks.newMock<IA>();
	mocks.RegisterExpectation(iamock, &IA::f);
	mocks.RegisterExpectation(iamock, &IA::g);
	mocks.ReplayAll();
	iamock->f();
	iamock->g();
	mocks.VerifyAll();
}

FUNC (checkExpectationsNotCompleted)
{
	bool exceptionCaught = false;
	MockRepository mocks;
	IA *iamock = mocks.newMock<IA>();
	mocks.RegisterExpectation(iamock, &IA::f);
	mocks.RegisterExpectation(iamock, &IA::g);
	mocks.ReplayAll();
	iamock->f();
	try {
		mocks.VerifyAll();
	}
	catch (ExpectationException &) 
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
}

FUNC (checkOvercompleteExpectations)
{
	MockRepository mocks;
	IA *iamock = mocks.newMock<IA>();
	mocks.RegisterExpectation(iamock, &IA::f);
	mocks.RegisterExpectation(iamock, &IA::g);
	mocks.ReplayAll();
	iamock->f();
	iamock->g();
	bool exceptionCaught = true;
	try {
		iamock->f();
	}
	catch (ExpectationException &) 
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
}

FUNC (checkExpectationsAreInOrder)
{
	MockRepository mocks;
	IA *iamock = mocks.newMock<IA>();
	mocks.RegisterExpectation(iamock, &IA::f);
	mocks.RegisterExpectation(iamock, &IA::g);
	mocks.ReplayAll();
	bool exceptionCaught = true;
	try {
		iamock->g();
	}
	catch (ExpectationException &) 
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
}

