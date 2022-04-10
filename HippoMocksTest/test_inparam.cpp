#include <string>
#include "hippomocks.h"
#include "gtest/gtest.h"

class IInParam { 
public:
	virtual ~IInParam() {}

	virtual void a(const std::string& in) = 0;
	virtual void b(std::string* in) = 0;
    virtual void c(std::string in) = 0;

    void a1() {
        this->a("Hello World");
    }

    void testC() {
        std::string in("Hello World");
        this->c(in);
    }
};

TEST (TestInParam, checkInParamsAreFilledIn_ConstChar)
{
	MockRepository mocks;
	IInParam *iamock = mocks.Mock<IInParam>();
	std::string teststring;
	mocks.ExpectCall(iamock, IInParam::a).With(In(teststring));
	
	iamock->a1();

	EXPECT_EQ(teststring, "Hello World");
}

TEST (TestInParam, checkInParamsAreFilledIn_StringByReference)
{
	MockRepository mocks;
	IInParam *iamock = mocks.Mock<IInParam>();
	std::string teststring;
	mocks.ExpectCall(iamock, IInParam::a).With(In(teststring));
	
	std::string in("Hello World");
	iamock->a(in);

	EXPECT_EQ(teststring, in);
}

TEST (TestInParam, checkInParamsAreFilledIn_PointerAddressMatch)
{
	MockRepository mocks;
	IInParam *iamock = mocks.Mock<IInParam>();
	std::string* teststring = NULL;
	mocks.ExpectCall(iamock, IInParam::b).With(In(teststring));
	
	std::string in("Hello World");
	iamock->b(&in);

	EXPECT_EQ(teststring, &in);
}


TEST (TestInParam, CheckInParamsAreFilled_StringByValue)
{
    MockRepository mocks;
    IInParam *iamock = mocks.Mock<IInParam>();
    std::string teststring = "";
    mocks.ExpectCall(iamock, IInParam::c).With(In(teststring));

    iamock->testC();

    EXPECT_EQ(teststring, "Hello World");
}
