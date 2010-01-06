#include <string>
#include "hippomocks.h"
#include "yaffut.h"

class IOutParam { 
public:
	virtual ~IOutParam() {}
	virtual void a(std::string& out) = 0;
};

FUNC (checkOutParamsAreFilledIn)
{
	MockRepository mocks;
	IOutParam *iamock = mocks.Mock<IOutParam>();
	mocks.ExpectCall(iamock, IOutParam::a).With(Out("Hello World"));
	
	std::string out;
	iamock->a(out);

	CHECK(out == "Hello World");
}
