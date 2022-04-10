#include <string>
#include "hippomocks.h"
#include "gtest/gtest.h"

class IDerefParam { 
public:
	virtual ~IDerefParam() {}
	virtual void a(std::string* in) = 0;
	virtual void b(const char* in) = 0;
	virtual void c(const int* in) = 0;
    void testA() {
		std::string in("Hello World");
		this->a(&in);
	};
	void testB() {
		const char* in = "Hello World";
		this->b(in);	
	};
	void testC() {
		int stack = 33;
		const int* in = &stack;
		this->c(in);
	};
};


TEST (TestDerefParam, checkDerefMatch_String)
{
	MockRepository mocks;
	IDerefParam *iamock = mocks.Mock<IDerefParam>();
	std::string teststring = "Hello World";
	mocks.ExpectCall(iamock, IDerefParam::a).With(Deref(teststring));

	iamock->testA();
}

TEST (TestDerefParam, checkDerefMatch_Char)
{
	MockRepository mocks;
	IDerefParam *iamock = mocks.Mock<IDerefParam>();
	mocks.ExpectCall(iamock, IDerefParam::b).With(Deref('H'));

	iamock->testB();
}

TEST (TestDerefParam, checkDerefMatch_Int)
{
	MockRepository mocks;
	IDerefParam *iamock = mocks.Mock<IDerefParam>();
	mocks.ExpectCall(iamock, IDerefParam::c).With(Deref(33));

	iamock->testC();
}