#include "hippomocks.h"
#include "yaffut.h"

class IM { 
public:
	virtual ~IM() {}
	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void a() = 0;
	virtual void b() = 0;
};

FUNC (checkTransactionStyleWorks)
{
	MockRepository mocks;
	IM *iamock = mocks.InterfaceMock<IM>();
	mocks.autoExpect = false;
	Call &beginCall = mocks.ExpectCall(iamock, IM::begin);
	Call &aCall = mocks.ExpectCall(iamock, IM::a).After(beginCall);
	Call &bCall = mocks.ExpectCall(iamock, IM::b).After(beginCall);
	mocks.ExpectCall(iamock, IM::end).After(aCall).After(bCall);
	iamock->begin();
	iamock->b();
	iamock->a();
	iamock->end();
}

FUNC (checkTransactionStyleFailIfOneSkipped)
{
	MockRepository mocks;
	IM *iamock = mocks.InterfaceMock<IM>();
	mocks.autoExpect = false;
	Call &beginCall = mocks.ExpectCall(iamock, IM::begin);
	Call &aCall = mocks.ExpectCall(iamock, IM::a).After(beginCall);
	Call &bCall = mocks.ExpectCall(iamock, IM::b).After(beginCall);
	mocks.ExpectCall(iamock, IM::end).After(aCall).After(bCall);
	iamock->begin();
	iamock->b();
	bool exceptionCaught = false;
	try {
		iamock->end();
	}
	catch (ExpectationException &) 
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
	mocks.reset();
}

