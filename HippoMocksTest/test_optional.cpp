#include "hippomocks.h"
#include "yaffut.h"

class II { 
public:
	virtual ~II() {}
	virtual void f() {}
	virtual void g() = 0;
};

FUNC (checkBaseCaseOptionalOk)
{
	MockRepository mocks;
	II *iamock = mocks.InterfaceMock<II>();
	mocks.ExpectCall(iamock, II::f);
	mocks.OnCall(iamock, II::g);
	iamock->g();
	iamock->f();
}

FUNC (checkBaseCaseOptionalNotRequired)
{
	MockRepository mocks;
	II *iamock = mocks.InterfaceMock<II>();
	mocks.ExpectCall(iamock, II::f);
	mocks.OnCall(iamock, II::g);
	iamock->f();
}

FUNC (checkBaseCaseOptionalTwiceIsOK)
{
	MockRepository mocks;
	II *iamock = mocks.InterfaceMock<II>();
	mocks.ExpectCall(iamock, II::f);
	mocks.OnCall(iamock, II::g);
	iamock->g();
	iamock->f();
	iamock->g();
}

