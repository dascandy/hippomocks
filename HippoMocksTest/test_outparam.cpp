#include <string>
#include "hippomocks.h"
#include "yaffut.h"

class IOutParam { 
public:
	virtual ~IOutParam() {}
	virtual void a(std::string& out) = 0;
	virtual void b(std::string** out) = 0;
	virtual void c(char** out) = 0;
};

FUNC (checkOutParamsAreFilledIn_ConstChar)
{
	MockRepository mocks;
	IOutParam *iamock = mocks.Mock<IOutParam>();
	mocks.ExpectCall(iamock, IOutParam::a).With(Out("Hello World"));
	
	std::string out;
	iamock->a(out);

	CHECK(out == "Hello World");
}

FUNC (checkOutParamsAreFilledIn_String)
{
	MockRepository mocks;
	IOutParam *iamock = mocks.Mock<IOutParam>();
	std::string teststring("Hello World");
	mocks.ExpectCall(iamock, IOutParam::a).With(Out(teststring));
	
	std::string out;
	iamock->a(out);

	CHECK(out == teststring);
}

FUNC (checkOutParamsAreFilledIn_PointerToString)
{
	MockRepository mocks;
	IOutParam *iamock = mocks.Mock<IOutParam>();
	std::string teststring("Hello World");
	mocks.ExpectCall(iamock, IOutParam::b).With(Out(new std::string(teststring)));
	
	std::string* out = 0;
	iamock->b(&out);

	CHECK(*out == teststring);
}

FUNC (checkOutParamsAreFilledIn_Char)
{
	MockRepository mocks;
	IOutParam *iamock = mocks.Mock<IOutParam>();
	const char* teststring = "Hello World";
	mocks.ExpectCall(iamock, IOutParam::c).With(Out((char*)teststring));
	
	char* out = 0;
	iamock->c(&out);

	CHECK(strcmp(out, teststring) == 0);
}
