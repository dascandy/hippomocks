#include "hippomocks.h"
#include "yaffut.h"
#include <utility>
using std::auto_ptr;

class X {};

class IQ {
public:
	virtual ~IQ() {}
	virtual auto_ptr<X> g();
	virtual auto_ptr<IQ> getSelf();
};

FUNC (checkAutoptrReturnable)
{
	X *_t = new X();

	MockRepository mocks;
	IQ *iamock = mocks.InterfaceMock<IQ>();
	mocks.ExpectCall(iamock, IQ::g).Return(auto_ptr<X>(_t));
	CHECK(_t == iamock->g().get());
}

FUNC (checkAutoptrCanReturnMock)
{
	MockRepository mocks;
	IQ *iamock = mocks.InterfaceMock<IQ>();
	mocks.ExpectCall(iamock, IQ::getSelf).Return(auto_ptr<IQ>(iamock));
	mocks.ExpectCallDestructor(iamock);
	CHECK(iamock == iamock->getSelf().get());
}

FUNC(checkCanDestroyMock)
{
	MockRepository mocks;
	IQ *iamock = mocks.InterfaceMock<IQ>();
	mocks.ExpectCallDestructor(iamock);
	delete iamock;
}

