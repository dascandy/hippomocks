#include "Framework.h"
#include "hippomocks.h"
#include <string>

class INice {
public:
    virtual ~INice() {}
    virtual void a() = 0;
    virtual void b() = 0;
    virtual void c() = 0;
};


TEST (checkThatUnexpectedCallsDoNotFail)
{
    MockRepository mocks;
    auto nice = mocks.NiceMock<INice>();
    nice->a();
    nice->b();
    nice->c();
}

TEST (checkThatUnexpectedCallsDoNotFailForUniquePtr)
{
    MockRepository mocks;
    auto nice = mocks.UniqueNiceMock<INice>();
    nice->a();
    nice->b();
    nice->c();
}

TEST (checkNeverCallWorksWithNiceMock)
{
    bool exceptionCaught = false;
    MockRepository mocks;
    auto nice = mocks.NiceMock<INice>();
    mocks.NeverCall(nice, INice::a);
    try
    {
        nice->a();
    }
    catch (HippoMocks::ExpectationException &)
    {
        exceptionCaught = true;
    }
    CHECK(exceptionCaught);
}
