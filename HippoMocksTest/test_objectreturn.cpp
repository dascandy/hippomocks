#include "hippomocks.h"
#include "gtest/gtest.h"

class Argument
{
public:
    virtual ~Argument() { };
    int value;
};

class ISS
{
public:
    virtual Argument getValue() = 0;
};

TEST (TestObjectReturn, checkUnexpectedCall)
{
    MockRepository mocks;

    ISS* is = mocks.Mock<ISS>();
    bool unexpectedCall = false;

    try
    {
        Argument arg = is->getValue();
    }
    catch (HippoMocks::NotImplementedException&)
    {
        unexpectedCall = true;
    }

    EXPECT_TRUE(unexpectedCall);
}
