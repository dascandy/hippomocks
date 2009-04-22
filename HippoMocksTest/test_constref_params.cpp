#include "hippomocks.h"
#include "yaffut.h"
#include <string>

class IM { 
public:
	virtual ~IM() {}
	virtual void e(const std::string &var) = 0;
};

FUNC (checkConstRefClassParam)
{
	MockRepository mocks;
	IM *iamock = mocks.InterfaceMock<IM>();
	mocks.ExpectCall(iamock, IM::e).With("Hello");
	iamock->e("Hello");
}

