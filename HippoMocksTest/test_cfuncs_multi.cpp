#include "hippomocks.h"
#include "Framework.h"
#include "target_cfuncs.h"

// If it's not supported, then don't test it.
#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT

TEST (checkMultiExpectedCall)
{
    MockRepository mocks;
    mocks.ExpectCallsFunc(ret_1, 2).Return(3);
    EQUALS(ret_1(), 3);
    EQUALS(ret_1(), 3);
}

TEST (checkMultiExpectCallWeaveOnCall)
{
    MockRepository mocks;
    mocks.ExpectCallsFunc(ret_1, 2).Return(3);
    mocks.OnCallFunc(ret_2).Return(4);
    EQUALS(ret_1(), 3);
    EQUALS(ret_2(), 4);
    EQUALS(ret_1(), 3);
}

TEST (checkMultiExpectCallWeaveExpectCall)
{
    MockRepository mocks;
    mocks.ExpectCallsFunc(ret_1, 2).Return(3);
    mocks.ExpectCallFunc(ret_2).Return(4);
    mocks.ExpectCallFunc(ret_1).Return(5);
    EQUALS(ret_1(), 3);
    EQUALS(ret_1(), 3);
    EQUALS(ret_2(), 4);
    EQUALS(ret_1(), 5);
}

TEST (checkMultiExpectedCallUnder)
{
    bool exceptionCaught = false;
    try {
        MockRepository mocks;
        mocks.ExpectCallsFunc(ret_1, 2).Return(3);
        EQUALS(ret_1(), 3);
    } catch (HippoMocks::CallMissingException) {
        exceptionCaught = true;
    }

    CHECK(exceptionCaught)
}

TEST (checkMultiExpectedCallOver)
{
    bool exceptionCaught = false;
    try {
        MockRepository mocks;
        mocks.ExpectCallsFunc(ret_1, 2).Return(3);
        EQUALS(ret_1(), 3);
        EQUALS(ret_1(), 3);
        EQUALS(ret_1(), 3);
    } catch (HippoMocks::ExpectationException) {
        exceptionCaught = true;
    }

    CHECK(exceptionCaught)
}


TEST (checkMultiExpectCallWeaveExpectCallException)
{
    bool exceptionCaught = false;
    try {
        MockRepository mocks;
        mocks.ExpectCallsFunc(ret_1, 2).Return(3);
        mocks.ExpectCallFunc(ret_2).Return(4);
        EQUALS(ret_1(), 3);
        EQUALS(ret_2(), 4);
        EQUALS(ret_1(), 3);
    } catch (HippoMocks::ExpectationException) {
        exceptionCaught = true;
    }

    CHECK(exceptionCaught)
}
#endif
