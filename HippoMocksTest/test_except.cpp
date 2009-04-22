#include "hippomocks.h"
#include "yaffut.h"

class IE {
public:
	virtual ~IE() {}
	virtual int f();
	virtual std::string g() = 0;
};

FUNC (checkPrimitiveExceptionAcceptedAndThrown)
{
	MockRepository mocks;
	IE *iamock = mocks.InterfaceMock<IE>();
	mocks.ExpectCall(iamock, IE::f).Throw(42);
	bool exceptionCaught = false;
	try 
	{
		iamock->f();
	}
	catch(int a)
	{
		CHECK(a == 42);
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
}

class SomeException : public std::exception {
private:
	const char *text;
public:
	SomeException(const char *txt) : text(txt) {}
	const char *what() const throw() { return text; }
};

FUNC (checkClassTypeExceptionWithContent)
{
	const char *sText = "someText";
	MockRepository mocks;
	IE *iamock = mocks.InterfaceMock<IE>();
	mocks.ExpectCall(iamock, IE::f).Throw(SomeException(sText));
	bool exceptionCaught = false;
	try 
	{
		iamock->f();
	}
	catch(SomeException &a)
	{
		CHECK(a.what() == sText);
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
}

FUNC(checkMockRepoVerifyDoesNotThrowDuringException)
{
	bool exceptionCaught = false;
	try
	{
		MockRepository mocks;
		IE *iamock = mocks.InterfaceMock<IE>();
		mocks.ExpectCall(iamock, IE::f);
	}
	catch (CallMissingException &)
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
	exceptionCaught = false;
	try
	{
		MockRepository mocks;
		IE *iamock = mocks.InterfaceMock<IE>();
		mocks.ExpectCall(iamock, IE::f);
		throw 42;
	}
	catch (int)
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
}

