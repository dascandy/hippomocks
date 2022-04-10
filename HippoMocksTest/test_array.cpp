#include "hippomocks.h"
#include "gtest/gtest.h"

class IArrayArgs {
public:
	virtual ~IArrayArgs() {}
	virtual void f(std::string);
	virtual void g(std::wstring) = 0;
};

TEST (TestArray, checkArrayArgumentsAccepted)
{
	MockRepository mocks;
	IArrayArgs *iamock = mocks.Mock<IArrayArgs>();
	mocks.ExpectCall(iamock, IArrayArgs::f).With("Hello");
	mocks.ExpectCall(iamock, IArrayArgs::g).With(L"World");
	iamock->f("Hello");
	iamock->g(L"World");
}


