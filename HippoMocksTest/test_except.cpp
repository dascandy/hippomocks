#include "hippomocks.h"
#include "gtest/gtest.h"

// For obvious reasons, the Throw is not present when you disable exceptions.
#ifndef HM_NO_EXCEPTIONS
class IE {
public:
	virtual ~IE() {}
	virtual int f();
	virtual std::string g() = 0;
};

TEST (TestExcept, checkPrimitiveExceptionAcceptedAndThrown)
{
	MockRepository mocks;
	IE *iamock = mocks.Mock<IE>();
	mocks.ExpectCall(iamock, IE::f).Throw(42);
	bool exceptionCaught = false;
	try 
	{
		iamock->f();
	}
	catch(int a)
	{
		EXPECT_EQ(a, 42);
		exceptionCaught = true;
	}
	EXPECT_TRUE(exceptionCaught);
}

class SomeException : public std::exception {
private:
	const char *text;
public:
	SomeException(const char *txt) : text(txt) {}
	const char *what() const throw() { return text; }
};

TEST (TestExcept, checkClassTypeExceptionWithContent)
{
	const char *sText = "someText";
	MockRepository mocks;
	IE *iamock = mocks.Mock<IE>();
	mocks.ExpectCall(iamock, IE::f).Throw(SomeException(sText));
	bool exceptionCaught = false;
	try 
	{
		iamock->f();
	}
	catch(SomeException &a)
	{
		EXPECT_EQ(a.what(), sText);
		exceptionCaught = true;
	}
	EXPECT_TRUE(exceptionCaught);
}

TEST (TestExcept, checkMockRepoVerifyDoesNotThrowDuringException)
{
	bool exceptionCaught = false;
	try
	{
		MockRepository mocks;
		IE *iamock = mocks.Mock<IE>();
		mocks.ExpectCall(iamock, IE::f);
	}
	catch (HippoMocks::CallMissingException &)
	{
		exceptionCaught = true;
	}
	EXPECT_TRUE(exceptionCaught);
	exceptionCaught = false;
	try
	{
		MockRepository mocks;
		IE *iamock = mocks.Mock<IE>();
		mocks.ExpectCall(iamock, IE::f);
		throw 42;
	}
	catch (int)
	{
		exceptionCaught = true;
	}
	EXPECT_TRUE(exceptionCaught);
}
#endif

