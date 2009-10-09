#include "hippomocks.h"
#include "yaffut.h"
#include <string>

class IR {
public:
	virtual void func() = 0;
};

class R : public IR {
public:
	virtual void func() {}
};

class IM { 
public:
	virtual ~IM() {}
	virtual void e(const std::string &var) = 0;
	virtual void f(const IR &arg) = 0;
};

FUNC (checkConstRefClassParam)
{
	MockRepository mocks;
	IM *iamock = mocks.InterfaceMock<IM>();
	mocks.ExpectCall(iamock, IM::e).With("Hello");
	iamock->e("Hello");
}

bool operator==(const IR &, const IR &)
{
	return true;
}

FUNC (checkConstRefAbstractClassParam)
{
	MockRepository mocks;
	IM *iamock = mocks.InterfaceMock<IM>();
	mocks.ExpectCall(iamock, IM::f).With(R());
	iamock->f(R());
}

