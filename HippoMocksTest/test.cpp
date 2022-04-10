#include "hippomocks.h"
#include "gtest/gtest.h"

class IA { 
public:
	virtual ~IA() {}
	virtual void f() {}
	virtual void g() = 0;
};

TEST (BasicTest, checkBaseCase)
{
	MockRepository mocks;
	IA *iamock = mocks.Mock<IA>();
	mocks.ExpectCall(iamock, IA::f);
	mocks.ExpectCall(iamock, IA::g);
	iamock->f();
	iamock->g();
}

TEST (BasicTest, checkMultiCall)
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

TEST (BasicTest, checkMultiCallNotCalled)
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
	EXPECT_TRUE(exceptionCaught);
}

TEST (BasicTest, checkMultiCallWrongOrder)
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
	EXPECT_TRUE(exceptionCaught);
	mocks.reset();
}

TEST (BasicTest, checkExpectationsNotCompleted)
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
	EXPECT_TRUE(exceptionCaught);
}

TEST (BasicTest, checkOvercompleteExpectations)
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
	EXPECT_TRUE(exceptionCaught);
}

TEST (BasicTest, checkExpectationsAreInOrder)
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
	EXPECT_TRUE(exceptionCaught);
}

