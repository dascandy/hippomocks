#include <string>
#include "hippomocks.h"
#include "gtest/gtest.h"

using HippoMocks::byRef;

class IDoFunctor { 
public:
	virtual ~IDoFunctor() {}
	virtual void a(std::string arg) = 0;
};

#if __cplusplus > 199711L
TEST (TestDo, checkDoFunctorCanBeLambdaWithContent)
{
	MockRepository mocks;
	IDoFunctor *iamock = mocks.Mock<IDoFunctor>();
  {
    std::string value = "Hello World";
    mocks.ExpectCall(iamock, IDoFunctor::a).Do([=](std::string arg){
      EXPECT_EQ(arg, value);
    });
  }

  iamock->a("Hello World");
}
#endif

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

TEST (TestDo, checkFunctorsCalled)
{
	MockRepository mocks;
	II *iamock = mocks.Mock<II>();
	mocks.ExpectCall(iamock, II::f).Do(setChecked);
	mocks.OnCall(iamock, II::g).Do(setChecked);
	checked = false;
	iamock->g();
	EXPECT_TRUE(checked);
	checked = false;
	iamock->f();
	EXPECT_TRUE(checked);
	checked = false;
	iamock->g();
	EXPECT_TRUE(checked);
}

TEST (TestDo, checkFunctorObjectCalled)
{
	MockRepository mocks;
	II *iamock = mocks.Mock<II>();
	functorClass obj;
	mocks.ExpectCall(iamock, II::f).Do(byRef(obj));
	mocks.OnCall(iamock, II::g).Do(byRef(obj));
	EXPECT_EQ(obj.calls, 0);
	iamock->g();
	EXPECT_EQ(obj.calls, 1);
	iamock->f();
	EXPECT_EQ(obj.calls, 2);
	iamock->g();
	EXPECT_EQ(obj.calls, 3);
}

class functorByVal {
public:
  functorByVal(int &count) : count(count) {}
  void operator()() { count++; }
  int &count;
private:
  functorByVal &operator=(const functorByVal&); // Silence VS
};

TEST (TestDo, checkFunctorObjectByvalCalled)
{
  int count = 0;
	MockRepository mocks;
	II *iamock = mocks.Mock<II>();
	mocks.ExpectCall(iamock, II::f).Do(functorByVal(count));
	mocks.OnCall(iamock, II::g).Do(functorByVal(count));
	EXPECT_EQ(count, 0);
	iamock->g();
	EXPECT_EQ(count, 1);
	iamock->f();
	EXPECT_EQ(count, 2);
	iamock->g();
	EXPECT_EQ(count, 3);
}


