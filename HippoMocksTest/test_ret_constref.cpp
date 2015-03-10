#include "hippomocks.h"
#include "Framework.h"

class Base {
public:
  virtual ~Base() {}
  virtual std::string foo() const = 0;
};

class Impl : public Base {
public:
  Impl(const std::string& value) : value(value) {}
  virtual std::string foo() const { return value; }
private:
  std::string value;
};

class Interface {
public:
	virtual ~Interface() {}
	virtual const Base& get() = 0;
};

TEST (checkRetvalConstrefAccepted)
{
  Impl impl("foo");
	MockRepository mocks;
	Interface *iamock = mocks.Mock<Interface>();
	mocks.ExpectCall(iamock, Interface::get).Return(impl);
	iamock->get();
}

TEST (checkRetvalConstrefProper)
{
  Impl impl("foo");
	MockRepository mocks;
	Interface *iamock = mocks.Mock<Interface>();
	mocks.ExpectCall(iamock, Interface::get).Return(impl);
	CHECK(iamock->get().foo() == "foo");
}

bool replace_get_called = false;
const Base& replace_get() 
{
  static Impl impl("bar");
	replace_get_called = true;
	return impl;
}

TEST (checkRetvalConstrefAfterDo)
{
  Impl impl("foo");
	MockRepository mocks;
	Interface *iamock = mocks.Mock<Interface>();
	mocks.ExpectCall(iamock, Interface::get).Do(replace_get).Return(impl);
	replace_get_called = false;
	CHECK(iamock->get().foo() == "foo");
	CHECK(replace_get_called);
}



