#include "Framework.h"
#include "hippomocks.h"
#include <string>

class IU {
public:
	virtual ~IU() { std::cout << "deleting  " << std::endl;}
	virtual void f() = 0;// {}
	virtual void g() {}
	virtual int h() { return 0; }
	virtual void i(std::string ) {}
	virtual void j(std::string ) = 0;
};

TEST (checkUniquePtrDestruction)
{
    MockRepository mocks;
    auto iu = mocks.UniqueMock<IU>();
}

TEST (checkCallsWorksOnUniquePtr)
{
    MockRepository mocks;
    std::unique_ptr<IU> iu = mocks.UniqueMock<IU>();
    mocks.ExpectCall(iu.get(), IU::f);
    iu->f();
}

TEST (checkMissingExpectationsWorksOnUniquePtr)
{
    MockRepository mocks;
    bool exceptionCaught = false;
    std::unique_ptr<IU> iu = mocks.UniqueMock<IU>();
    try
    {
        iu->f();
    }
    catch (HippoMocks::NotImplementedException const& e)
    {
        exceptionCaught = true;
    }
    CHECK(exceptionCaught);
}

TEST (checkNeverCallWorksOnUniquePtr)
{
	bool exceptionCaught = false;
	MockRepository mocks;
        auto iu = mocks.UniqueMock<IU>();
	Call &callF = mocks.ExpectCall(iu.get(), IU::f);
	mocks.OnCall(iu.get(), IU::g);
	mocks.NeverCall(iu.get(), IU::g).After(callF);
	iu->g();
	iu->g();
	iu->f();
	try
	{
		iu->g();
	}
	catch (HippoMocks::ExpectationException &)
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
}

TEST (checkClassArgumentsAcceptedWithUniquePtr)
{
	MockRepository mocks;
	auto iamock = mocks.UniqueMock<IU>();
	mocks.OnCall(iamock.get(), IU::i).With("hi");
	mocks.OnCall(iamock.get(), IU::j).With("bye");
	iamock->i("hi");
	iamock->j("bye");
}

TEST (checkClassArgumentsCheckedWithUniquePtr)
{
	MockRepository mocks;
	auto iamock = mocks.UniqueMock<IU>();
	mocks.OnCall(iamock.get(), IU::i).With("hi");
	mocks.OnCall(iamock.get(), IU::j).With("bye");
	bool exceptionCaught = false;
	try
	{
		iamock->i("bye");
	}
	catch (HippoMocks::ExpectationException)
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
	mocks.reset();
}

TEST (checkClassArgumentsIgnoredWithUniquePtr)
{
	MockRepository mocks;
        auto iamock = mocks.UniqueMock<IU>();
	mocks.OnCall(iamock.get(), IU::i);
	iamock->i("bye");
}
