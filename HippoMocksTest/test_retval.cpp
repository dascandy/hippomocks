#include "hippomocks.h"
#include "yaffut.h"

class ID {
public:
	virtual ~ID() {}
	virtual int f();
	virtual std::string g() = 0;
};

FUNC (checkRetvalAccepted)
{
	MockRepository mocks;
	ID *iamock = mocks.InterfaceMock<ID>();
	mocks.ExpectCall(iamock, &ID::f).Return(1);
	mocks.ExpectCall(iamock, &ID::g).Return("fsck");
	mocks.ReplayAll();
	iamock->f();
	iamock->g();
	mocks.VerifyAll();
}

FUNC (checkRetvalProper)
{
	MockRepository mocks;
	ID *iamock = mocks.InterfaceMock<ID>();
	mocks.ExpectCall(iamock, &ID::f).Return(1);
	mocks.ExpectCall(iamock, &ID::g).Return("fsck");
	mocks.ReplayAll();
	CHECK(iamock->f() == 1);
	CHECK(iamock->g() == "fsck");
}

