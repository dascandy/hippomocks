#include "hippomocks.h"
#include "gtest/gtest.h"
#include <string>

// non-reference is bad, but should work
class IC {
public:
	virtual ~IC() {}
	virtual void f(std::string s);
	virtual void g(std::string s) = 0;
};

TEST (TestClassArgs, checkClassArgumentsAccepted)
{
	MockRepository mocks;
	IC *iamock = mocks.Mock<IC>();
	mocks.OnCall(iamock, IC::f).With("hi");
	mocks.OnCall(iamock, IC::g).With("bye");
	iamock->f("hi");
	iamock->g("bye");
}

TEST (TestClassArgs, checkClassArgumentsChecked)
{
	MockRepository mocks;
	IC *iamock = mocks.Mock<IC>();
	mocks.OnCall(iamock, IC::f).With("hi");
	mocks.OnCall(iamock, IC::g).With("bye");
	bool exceptionCaught = false;
	try 
	{
		iamock->f("bye");
	}
	catch (HippoMocks::ExpectationException)
	{
		exceptionCaught = true;
	}
	EXPECT_TRUE(exceptionCaught);
	mocks.reset();
}

TEST (TestClassArgs, checkClassArgumentsIgnored)
{
	MockRepository mocks;
	IC *iamock = mocks.Mock<IC>();
	mocks.OnCall(iamock, IC::f);
	iamock->f("bye");
}

