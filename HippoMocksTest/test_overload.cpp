#include "hippomocks.h"
#include "yaffut.h"

class IA { 
public:
	virtual ~IA() {}
	virtual void f() = 0;
	virtual void f(int) = 0;
};

typedef void (IA::*mf)();

FUNC (checkOverloadWithCastWorks)
{
	MockRepository mocks;
	IA *iamock = mocks.InterfaceMock<IA>();
	mocks.ExpectCallOverload(iamock, (mf)&IA::f);
	iamock->f();
}

