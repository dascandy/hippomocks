#include <string>
#include "hippomocks.h"
#include "gtest/gtest.h"

class IOutParam { 
public:
	virtual ~IOutParam() {}
	virtual void a(std::string& out) = 0;
	virtual void b(std::string** out) = 0;
	virtual void c(char** out) = 0;
};

TEST (TestOutParam, checkOutParamsAreFilledIn_ConstChar)
{
	MockRepository mocks;
	IOutParam *iamock = mocks.Mock<IOutParam>();
	mocks.ExpectCall(iamock, IOutParam::a).With(Out("Hello World"));
	
	std::string out;
	iamock->a(out);

	EXPECT_EQ(out, "Hello World");
}

TEST (TestOutParam, checkOutParamsAreFilledIn_String)
{
	MockRepository mocks;
	IOutParam *iamock = mocks.Mock<IOutParam>();
	std::string teststring("Hello World");
	mocks.ExpectCall(iamock, IOutParam::a).With(Out(teststring));
	
	std::string out;
	iamock->a(out);

	EXPECT_EQ(out, teststring);
}

TEST (TestOutParam, checkOutParamsAreFilledIn_PointerToString)
{
	MockRepository mocks;
	IOutParam *iamock = mocks.Mock<IOutParam>();
	std::string teststring("Hello World");
  std::string *outString = new std::string(teststring);
	mocks.ExpectCall(iamock, IOutParam::b).With(Out(outString));
	
	std::string* out = 0;
	iamock->b(&out);

	EXPECT_EQ(*out, teststring);

  delete outString;
}

TEST (TestOutParam, checkOutParamsAreFilledIn_Char)
{
	MockRepository mocks;
	IOutParam *iamock = mocks.Mock<IOutParam>();
	const char* teststring = "Hello World";
	mocks.ExpectCall(iamock, IOutParam::c).With(Out((char*)teststring));
	
	char* out = 0;
	iamock->c(&out);

	EXPECT_EQ(strcmp(out, teststring), 0);
}
