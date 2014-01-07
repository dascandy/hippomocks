#include "hippomocks.h"
#include "Framework.h"

class II { 
public:
	virtual ~II() {}
	virtual void f() {}
	virtual void g() = 0;
};

bool checked;
void setChecked() { checked = true; }

class functorClass {
public:
	functorClass() : calls(0) {}
	int calls;
	void operator()() { ++calls; }
};

TEST (checkFunctorsCalled)
{
	MockRepository mocks;
	II *iamock = mocks.Mock<II>();
	mocks.ExpectCall(iamock, II::f).Do(setChecked);
	mocks.OnCall(iamock, II::g).Do(setChecked);
	checked = false;
	iamock->g();
	CHECK(checked == true);
	checked = false;
	iamock->f();
	CHECK(checked == true);
	checked = false;
	iamock->g();
	CHECK(checked == true);
}

TEST (checkFunctorObjectCalled)
{
	MockRepository mocks;
	II *iamock = mocks.Mock<II>();
	functorClass obj;
	mocks.ExpectCall(iamock, II::f).Do(obj);
	mocks.OnCall(iamock, II::g).Do(obj);
	CHECK(obj.calls == 0);
	iamock->g();
	CHECK(obj.calls == 1);
	iamock->f();
	CHECK(obj.calls == 2);
	iamock->g();
	CHECK(obj.calls == 3);
}

