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
	mocks.ExpectCall(iamock, &IE::f).Throw(42);
	mocks.ReplayAll();
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
	mocks.VerifyAll();
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
	mocks.ExpectCall(iamock, &IE::f).Throw(SomeException(sText));
	mocks.ReplayAll();
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
	mocks.VerifyAll();
}

