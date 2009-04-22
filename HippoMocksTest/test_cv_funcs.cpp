#include "hippomocks.h"
#include "yaffut.h"

class IM { 
public:
	virtual ~IM() {}
	virtual void e() = 0;
	virtual void f() const = 0;
	virtual void g() volatile = 0;
	virtual void h() const volatile = 0;
};

FUNC (checkCVQualifiedMemberFunctions)
{
	MockRepository mocks;
	IM *iamock = mocks.InterfaceMock<IM>();
	mocks.ExpectCall(iamock, IM::e);
	mocks.ExpectCall(iamock, IM::f);
	mocks.ExpectCall(iamock, IM::g);
	mocks.ExpectCall(iamock, IM::h);
	iamock->e();
	iamock->f();
	iamock->g();
	iamock->h();
}

