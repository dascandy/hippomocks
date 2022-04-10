#include "hippomocks.h"
#include "gtest/gtest.h"
#include <utility>
using std::auto_ptr;

class X {};

class IQ {
public:
	virtual ~IQ() {}
	virtual void f();
	virtual auto_ptr<X> g();
	virtual auto_ptr<IQ> getSelf();
};

TEST (TestAutoPtr, checkAutoptrReturnable)
{
	X *_t = new X;

	MockRepository mocks;
	IQ *iamock = mocks.Mock<IQ>();
	mocks.ExpectCall(iamock, IQ::g).Return(auto_ptr<X>(_t));
	EXPECT_EQ(_t, iamock->g().get());
}

TEST (TestAutoPtr, checkAutoptrCanReturnMock)
{
	MockRepository mocks;
	IQ *iamock = mocks.Mock<IQ>();
	mocks.ExpectCall(iamock, IQ::getSelf).Return(auto_ptr<IQ>(iamock));
	mocks.ExpectCallDestructor(iamock);
	EXPECT_EQ(iamock, iamock->getSelf().get());
}

TEST (TestAutoPtr, checkCanDestroyMock)
{
	MockRepository mocks;
	IQ *iamock = mocks.Mock<IQ>();
	mocks.ExpectCallDestructor(iamock);
	delete iamock;
}

TEST (TestAutoPtr, checkAutoptrStability)
{
	int exceptionsCaught = 0;
	try
	{
		MockRepository mocks;
		try
		{
			IQ *iamock = mocks.Mock<IQ>();
			auto_ptr<IQ> auto_ptr(iamock);
			mocks.ExpectCall(iamock, IQ::f);
			mocks.ExpectCallDestructor(iamock);
			throw 42;
			// implicit: destructor for iamock, that may not throw
		}
		catch(int)
		{
			exceptionsCaught++;
		}
	}
	catch(HippoMocks::ExpectationException)
	{
		exceptionsCaught++;
	}
	EXPECT_EQ(2, exceptionsCaught);
}

