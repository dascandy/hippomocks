#include "hippomocks.h"
#include "yaffut.h"

class IF {
public:
	virtual ~IF() {}
	virtual void f(int) {}
	virtual void g(int) {}
	std::string name;
};

FUNC (checkClassMockBasicallyWorks)
{
	MockRepository mocks;
	IF *iamock = mocks.ClassMock<IF>();
	mocks.ExpectCall(iamock, IF::f).With(1);
	mocks.ExpectCall(iamock, IF::g).With(2);
	iamock->name = "hey";
	iamock->f(1);
	iamock->g(2);
	CHECK(iamock->name == "hey");
}

class IG { 
public:
	IF obj;
	virtual int h() { return 0; }
};

FUNC (checkComplexClassMockWorks)
{
	MockRepository mocks;
	IG *igmock = mocks.ClassMock<IG>();
	mocks.ExpectCall(igmock, IG::h).Return(42);
	igmock->obj.f(4);
	CHECK(igmock->h() == 42);
}

