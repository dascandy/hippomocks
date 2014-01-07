#include "hippomocks.h"
#include "Framework.h"
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

TEST (checkConstRefClassParam)
{
	MockRepository mocks;
	IM *iamock = mocks.Mock<IM>();
	mocks.ExpectCall(iamock, IM::e).With("Hello");
	iamock->e("Hello");
}

bool operator==(const IR &, const IR &)
{
	return true;
}

TEST (checkConstRefAbstractClassParam)
{
	R r;
	MockRepository mocks;
	IM *iamock = mocks.Mock<IM>();
	mocks.ExpectCall(iamock, IM::f).With(r);
	iamock->f(r);
}

