#include "hippomocks.h"
#include "gtest/gtest.h"

class ID {
public:
	virtual ~ID() {}
	virtual int f();
	virtual std::string g() = 0;
};

TEST (TestRetVal, checkRetvalAccepted)
{
	MockRepository mocks;
	ID *iamock = mocks.Mock<ID>();
	mocks.ExpectCall(iamock, ID::f).Return(1);
	mocks.ExpectCall(iamock, ID::g).Return("fsck");
	iamock->f();
	iamock->g();
}

TEST (TestRetVal, checkRetvalProper)
{
	MockRepository mocks;
	ID *iamock = mocks.Mock<ID>();
	mocks.ExpectCall(iamock, ID::f).Return(1);
	mocks.ExpectCall(iamock, ID::g).Return("fsck");
	EXPECT_EQ(iamock->f(), 1);
	EXPECT_EQ(iamock->g(), "fsck");
}

bool replace_g_called = false;
std::string replace_g() 
{
	replace_g_called = true;
	return "";
}

TEST (TestRetVal, checkRetvalAfterDo)
{
	MockRepository mocks;
	ID *iamock = mocks.Mock<ID>();
	mocks.ExpectCall(iamock, ID::g).Do(replace_g).Return("fsck");
	replace_g_called = false;
	EXPECT_EQ(iamock->g(), "fsck");
	EXPECT_TRUE(replace_g_called);
}


