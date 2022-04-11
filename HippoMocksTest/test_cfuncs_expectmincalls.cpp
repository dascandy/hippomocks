#include "hippomocks.h"
#include "gtest/gtest.h"
#include "target_cfuncs.h"

// If it's not supported, then don't test it.
#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT

TEST (TestCfuncsExpectMinCalls, checkExpectMinCalls)
{
    MockRepository mocks;
    mocks.ExpectMinCallsFunc(ret_1, 2).Return(3);
    EXPECT_EQ(ret_1(), 3);
    EXPECT_EQ(ret_1(), 3);
}

TEST (TestCfuncsExpectMinCalls, checkExpectMinCallsOver)
{

    MockRepository mocks;
    mocks.ExpectMinCallsFunc(ret_1, 2).Return(3);
    EXPECT_EQ(ret_1(), 3);
    EXPECT_EQ(ret_1(), 3);
    EXPECT_EQ(ret_1(), 3);
    EXPECT_EQ(ret_1(), 3);
    EXPECT_EQ(ret_1(), 3);
}

TEST (TestCfuncsExpectMinCalls, checkExpectMinCallsWeaveOnCall)
{
    MockRepository mocks;
    mocks.ExpectMinCallsFunc(ret_1, 2).Return(3);
    mocks.OnCallFunc(ret_2).Return(4);
    EXPECT_EQ(ret_1(), 3);
    EXPECT_EQ(ret_2(), 4);
    EXPECT_EQ(ret_1(), 3);
}

TEST (TestCfuncsExpectMinCalls, checkExpectMinCallsWeaveExpectCall)
{
    MockRepository mocks;
    mocks.ExpectMinCallsFunc(ret_1, 2).Return(3);
    mocks.ExpectCallFunc(ret_2).Return(4);
    mocks.ExpectCallFunc(ret_1).Return(5);
    EXPECT_EQ(ret_1(), 3);
    EXPECT_EQ(ret_1(), 3);
    EXPECT_EQ(ret_2(), 4);
    EXPECT_EQ(ret_1(), 5);
}

TEST (TestCfuncsExpectMinCalls, checkExpectMinCallsUnder)
{
    bool exceptionCaught = false;
    try {
        MockRepository mocks;
        mocks.ExpectMinCallsFunc(ret_1, 2).Return(3);
        EXPECT_EQ(ret_1(), 3);
    } catch (HippoMocks::CallMissingException) {
        exceptionCaught = true;
    }

    EXPECT_TRUE(exceptionCaught);
}

TEST (TestCfuncsExpectMinCalls, checkExpectMinCallsWeaveExpectCallException)
{
    bool exceptionCaught = false;
    try {
        MockRepository mocks;
        mocks.ExpectMinCallsFunc(ret_1, 2).Return(3);
        mocks.ExpectCallFunc(ret_2).Return(4);
        EXPECT_EQ(ret_1(), 3);
        EXPECT_EQ(ret_2(), 4);
        EXPECT_EQ(ret_1(), 3);
    } catch (HippoMocks::ExpectationException) {
        exceptionCaught = true;
    }

    EXPECT_TRUE(exceptionCaught);
}
#endif
