#include "yaffut.h"
#include "hippomocks.h"

struct X {};

class IS {
public:
	virtual ~IS() {}
	virtual int f() { return 1; }
	virtual int g(int, int) { return 2; }
	virtual void h() = 0;
	virtual void i(int, const X &) = 0;
};

FUNC (checkNoResultContainsFuncName)
{
	bool exceptionCaught = false;
	MockRepository mocks;
	IS *iamock = mocks.InterfaceMock<IS>();
	mocks.ExpectCall(iamock, IS::f);
	mocks.OnCall(iamock, IS::g);
	try {
		iamock->f();
	} catch(NoResultSetUpException &ex) {
		exceptionCaught = true;
		CHECK(strstr(ex.what(), "IS::f") != NULL);
		CHECK(strlen(ex.what()) < strlen(__FILE__) + 80);
	}
	CHECK(exceptionCaught);
}

FUNC (checkNoResultContainsBlankArgSpec)
{
	bool exceptionCaught = false;
	MockRepository mocks;
	IS *iamock = mocks.InterfaceMock<IS>();
	mocks.ExpectCall(iamock, IS::g);
	mocks.OnCall(iamock, IS::f);
	try {
		iamock->g(1,2);
	} catch(NoResultSetUpException &ex) {
		exceptionCaught = true;
		CHECK(strstr(ex.what(), "IS::g") != NULL);
		CHECK(strstr(ex.what(), "(...)") != NULL);
		CHECK(strlen(ex.what()) < strlen(__FILE__) + 80);
	}
	CHECK(exceptionCaught);
}

FUNC (checkNoResultContainsActualArgSpec)
{
	bool exceptionCaught = false;
	MockRepository mocks;
	IS *iamock = mocks.InterfaceMock<IS>();
	mocks.ExpectCall(iamock, IS::g).With(1,2);
	mocks.OnCall(iamock, IS::f);
	try {
		iamock->g(1,2);
	} catch(NoResultSetUpException &ex) {
		exceptionCaught = true;
		CHECK(strstr(ex.what(), "IS::g") != NULL);
		CHECK(strstr(ex.what(), "(1,2)") != NULL);
		CHECK(strlen(ex.what()) < strlen(__FILE__) + 80);
	}
	CHECK(exceptionCaught);
}

FUNC (checkNoResultContainsActualUnprintableArgSpec)
{
	bool exceptionCaught = false;
	MockRepository mocks;
	IS *iamock = mocks.InterfaceMock<IS>();
	mocks.NeverCall(iamock, IS::i).With(42, X());
	try {
		iamock->h();
	} catch(NotImplementedException &ex) {
		exceptionCaught = true;
		CHECK(strstr(ex.what(), "IS::i") != NULL);
		CHECK(strstr(ex.what(), "(42,??\?)") != NULL);
	}
	CHECK(exceptionCaught);
}

FUNC (checkNoResultDoesNotComplainIfNotCalled)
{
	MockRepository mocks;
	IS *iamock = mocks.InterfaceMock<IS>();
	mocks.OnCall(iamock, IS::g).With(1,2);
	mocks.OnCall(iamock, IS::g).With(3,4).Return(42);
	iamock->g(3,4);
}

FUNC(checkNotImplementedExceptionToContainInfo)
{
	MockRepository mocks;
	mocks.autoExpect = false;
	IS *ismock = mocks.InterfaceMock<IS>();
	mocks.ExpectCall(ismock, IS::f).With().Return(1);
	mocks.OnCall(ismock, IS::f).Return(2);
	mocks.ExpectCall(ismock, IS::g).With(1,2).Return(2);
	mocks.OnCall(ismock, IS::g).Return(3);
	mocks.NeverCall(ismock, IS::g).With(3,4);

	try
	{
		ismock->h();
	}
	catch (NotImplementedException &ex)
	{
		CHECK(strstr(ex.what(), "Expectation for IS::f()") != NULL);
		CHECK(strstr(ex.what(), "Result set for IS::f(...)") != NULL);
		CHECK(strstr(ex.what(), "Expectation for IS::g(1,2)") != NULL);
		CHECK(strstr(ex.what(), "Result set for IS::g(...)") != NULL);
		CHECK(strstr(ex.what(), "Result set for IS::g(3,4)") != NULL);
		CHECK(strstr(ex.what(), __FILE__) != NULL);
	}
	mocks.reset();
}

FUNC(checkExpectationExceptionToContainInfo)
{
	MockRepository mocks;
	mocks.autoExpect = false;
	IS *ismock = mocks.InterfaceMock<IS>();
	mocks.ExpectCall(ismock, IS::f).With().Return(1);
	mocks.OnCall(ismock, IS::f).Return(2);
	mocks.ExpectCall(ismock, IS::g).With(1,2).Return(2);
	mocks.OnCall(ismock, IS::g).With(2,3).Return(3);
	mocks.NeverCall(ismock, IS::g).With(3,4);
	mocks.NeverCall(ismock, IS::h).With();

	try
	{
		ismock->g(0,1);
	}
	catch (ExpectationException &ex)
	{
		CHECK(strstr(ex.what(), "Function IS::g(0,1) called") != NULL);
		CHECK(strstr(ex.what(), "Expectation for IS::f()") != NULL);
		CHECK(strstr(ex.what(), "Result set for IS::f(...)") != NULL);
		CHECK(strstr(ex.what(), "Expectation for IS::g(1,2)") != NULL);
		CHECK(strstr(ex.what(), "Result set for IS::g(2,3)") != NULL);
		CHECK(strstr(ex.what(), "Result set for IS::g(3,4)") != NULL);
		CHECK(strstr(ex.what(), "Result set for IS::h()") != NULL);
		CHECK(strstr(ex.what(), __FILE__) != NULL);
	}
	mocks.reset();
}

FUNC(checkCallMissingExceptionToContainInfo)
{
	try
	{
		MockRepository mocks;
		mocks.autoExpect = false;
		IS *ismock = mocks.InterfaceMock<IS>();
		mocks.ExpectCall(ismock, IS::f).With().Return(1);
		mocks.OnCall(ismock, IS::f).Return(2);
		mocks.ExpectCall(ismock, IS::g).With(1,2).Return(2);
		mocks.OnCall(ismock, IS::g).Return(3);
		mocks.NeverCall(ismock, IS::g).With(3,4);
	}
	catch (CallMissingException &ex)
	{
		CHECK(strstr(ex.what(), "Expectation for IS::f()") != NULL);
		CHECK(strstr(ex.what(), "Result set for IS::f(...)") != NULL);
		CHECK(strstr(ex.what(), "Expectation for IS::g(1,2)") != NULL);
		CHECK(strstr(ex.what(), "Result set for IS::g(...)") != NULL);
		CHECK(strstr(ex.what(), "Result set for IS::g(3,4)") != NULL);
		CHECK(strstr(ex.what(), __FILE__) != NULL);
	}
}

