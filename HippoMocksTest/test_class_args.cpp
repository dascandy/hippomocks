#include "hippomocks.h"
#include "yaffut.h"
#include <string>

// non-reference is bad, but should work
class IC {
public:
	virtual ~IC() {}
	virtual void f(std::string s);
	virtual void g(std::string s) = 0;
};

FUNC (checkClassArgumentsAccepted)
{
	MockRepository mocks;
	IC *iamock = mocks.newMock<IC>();
	mocks.OnCall(iamock, &IC::f).With("hi");
	mocks.OnCall(iamock, &IC::g).With("bye");
	mocks.ReplayAll();
	iamock->f("hi");
	iamock->g("bye");
	mocks.VerifyAll();
}

FUNC (checkClassArgumentsChecked)
{
	MockRepository mocks;
	IC *iamock = mocks.newMock<IC>();
	mocks.OnCall(iamock, &IC::f).With("hi");
	mocks.OnCall(iamock, &IC::g).With("bye");
	mocks.ReplayAll();
	bool exceptionCaught = false;
	try 
	{
		iamock->f("bye");
	}
	catch (ExpectationException)
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
}

FUNC (checkClassArgumentsIgnored)
{
	MockRepository mocks;
	IC *iamock = mocks.newMock<IC>();
	mocks.OnCall(iamock, &IC::f);
	mocks.ReplayAll();
	iamock->f("bye");
}

