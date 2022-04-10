#include "gtest/gtest.h"
#include "hippomocks.h"

class IAA { 
public:
	virtual int f(int a, int b, int c);
};

bool allEven(int a, int b, int c) { return (a % 2 == 0) && (b % 2 == 0) && (c % 2 == 0); }
bool oeo(int a, int b, int c) { return (a % 2 == 1) && (b % 2 == 0) && (c % 2 == 1); }
bool eod(int a, int b, int) { return (a % 2 == 0) && (b % 2 == 1); }
bool never(int, int, int) { return false; }

TEST (TestFilter, checkFilterIsApplied)
{
	MockRepository mocks;
	IAA *iamock = mocks.Mock<IAA>();
	mocks.OnCall(iamock, IAA::f).Return(4);
	mocks.OnCall(iamock, IAA::f).Match(never).Return(3);
	mocks.OnCall(iamock, IAA::f).Match(eod).Return(2);
	mocks.OnCall(iamock, IAA::f).Match(oeo).Return(1);
	mocks.OnCall(iamock, IAA::f).Match(allEven).Return(5);
	EXPECT_EQ(5, iamock->f(0, 0, 0));
	EXPECT_EQ(4, iamock->f(0, 0, 1));
	EXPECT_EQ(2, iamock->f(0, 1, 0));
	EXPECT_EQ(2, iamock->f(0, 1, 1));
	EXPECT_EQ(4, iamock->f(1, 0, 0));
	EXPECT_EQ(1, iamock->f(1, 0, 1));
	EXPECT_EQ(4, iamock->f(1, 1, 0));
	EXPECT_EQ(4, iamock->f(1, 1, 1));
}

