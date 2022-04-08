#include <string>
#include "hippomocks.h"
#include "Framework.h"

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

TEST (checkInParamsAreFilledIn_ConstChar)
{
	MockRepository mocks;
	IInParam *iamock = mocks.Mock<IInParam>();
	std::string teststring;
	mocks.ExpectCall(iamock, IInParam::a).With(In(teststring));
	
	iamock->a1();

	CHECK(teststring == "Hello World");
}

TEST (checkInParamsAreFilledIn_StringByReference)
{
	MockRepository mocks;
	IInParam *iamock = mocks.Mock<IInParam>();
	std::string teststring;
	mocks.ExpectCall(iamock, IInParam::a).With(In(teststring));
	
	std::string in("Hello World");
	iamock->a(in);

	CHECK(teststring == in);
}

TEST (checkInParamsAreFilledIn_PointerAddressMatch)
{
	MockRepository mocks;
	IInParam *iamock = mocks.Mock<IInParam>();
	std::string* teststring = NULL;
	mocks.ExpectCall(iamock, IInParam::b).With(In(teststring));
	
	std::string in("Hello World");
	iamock->b(&in);

	CHECK(teststring == &in);
}


TEST (CheckInParamsAreFilled_StringByValue)
{
    MockRepository mocks;
    IInParam *iamock = mocks.Mock<IInParam>();
    std::string teststring = "";
    mocks.ExpectCall(iamock, IInParam::c).With(In(teststring));

    iamock->testC();

    CHECK(teststring == "Hello World");
}
