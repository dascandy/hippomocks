#include "hippomocks.h"
#include "gtest/gtest.h"
#include "target_cfuncs.h"

// If it's not supported, then don't test it.
#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT

TEST (TestCfuncsExpectCalls, checkExpectCalls)
{
    MockRepository mocks;
    mocks.ExpectCallsFunc(ret_1, 2).Return(3);
    EXPECT_EQ(ret_1(), 3);
    EXPECT_EQ(ret_1(), 3);
}

TEST (TestCfuncsExpectCalls, checkExpectCallsWeaveOnCall)
{
    MockRepository mocks;
    mocks.ExpectCallsFunc(ret_1, 2).Return(3);
    mocks.OnCallFunc(ret_2).Return(4);
    EXPECT_EQ(ret_1(), 3);
    EXPECT_EQ(ret_2(), 4);
    EXPECT_EQ(ret_1(), 3);
}

TEST (TestCfuncsExpectCalls, checkExpectCallsWeaveExpectCall)
{
    MockRepository mocks;
    mocks.ExpectCallsFunc(ret_1, 2).Return(3);
    mocks.ExpectCallFunc(ret_2).Return(4);
    mocks.ExpectCallFunc(ret_1).Return(5);
    EXPECT_EQ(ret_1(), 3);
    EXPECT_EQ(ret_1(), 3);
    EXPECT_EQ(ret_2(), 4);
    EXPECT_EQ(ret_1(), 5);
}

TEST (TestCfuncsExpectCalls, checkExpectCallsUnder)
{
    bool exceptionCaught = false;
    try {
        MockRepository mocks;
        mocks.ExpectCallsFunc(ret_1, 2).Return(3);
        EXPECT_EQ(ret_1(), 3);
    } catch (HippoMocks::CallMissingException) {
        exceptionCaught = true;
    }

    EXPECT_TRUE(exceptionCaught);
}

TEST (TestCfuncsExpectCalls, checkExpectCallsOver)
{
    bool exceptionCaught = false;
    try {
        MockRepository mocks;
        mocks.ExpectCallsFunc(ret_1, 2).Return(3);
        EXPECT_EQ(ret_1(), 3);
        EXPECT_EQ(ret_1(), 3);
        EXPECT_EQ(ret_1(), 3);
    } catch (HippoMocks::ExpectationException) {
        exceptionCaught = true;
    }

    EXPECT_TRUE(exceptionCaught);
}


TEST (TestCfuncsExpectCalls, checkExpectCallsWeaveExpectCallException)
{
    bool exceptionCaught = false;
    try {
        MockRepository mocks;
        mocks.ExpectCallsFunc(ret_1, 2).Return(3);
        mocks.ExpectCallFunc(ret_2).Return(4);
        EXPECT_EQ(ret_1(), 3);
        EXPECT_EQ(ret_2(), 4);
        EXPECT_EQ(ret_1(), 3);
    } catch (HippoMocks::ExpectationException) {
        exceptionCaught = true;
    }

    EXPECT_TRUE(exceptionCaught);
}


TEST (TestCfuncsExpectCalls, checkExpectCallOverridesNeverCall) {
	MockRepository mocks;
	mocks.NeverCallFunc(ret_1);
	mocks.ExpectCallFunc(ret_1).Return(3);
	ret_1();
}

TEST (TestCfuncsExpectCalls, checkExpectCallOverridesNeverCallVoid) {
	MockRepository mocks;
	mocks.NeverCallFunc(ret_3);
	mocks.ExpectCallFunc(ret_3);
	ret_3();
}

#endif
