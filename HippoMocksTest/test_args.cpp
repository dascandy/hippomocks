#include "hippomocks.h"
#include "yaffut.h"

class IB {
public:
	virtual ~IB() {}
	virtual void f(int);
	virtual void g(int) = 0;
};

FUNC (checkArgumentsAccepted)
{
	MockRepository mocks;
	IB *iamock = mocks.InterfaceMock<IB>();
	mocks.ExpectCall(iamock, IB::f).With(1);
	mocks.ExpectCall(iamock, IB::g).With(2);
	iamock->f(1);
	iamock->g(2);
}

FUNC (checkArgumentsChecked)
{
	MockRepository mocks;
	IB *iamock = mocks.InterfaceMock<IB>();
	mocks.ExpectCall(iamock, IB::f).With(1);
	mocks.ExpectCall(iamock, IB::g).With(1);
	bool exceptionCaught = false;
	try 
	{
		iamock->f(2);
	}
	catch (ExpectationException)
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
	mocks.reset();
}

