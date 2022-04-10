#include "hippomocks.h"
#include "gtest/gtest.h"

class IZombie { 
public:
	virtual ~IZombie() {}
	virtual void a() = 0;
};

TEST (TestZombie, checkZombieCallsAreReported)
{
	bool exceptionCaught = false;
	MockRepository mocks;
	IZombie *iamock = mocks.Mock<IZombie>();
	mocks.ExpectCall(iamock, IZombie::a);
	mocks.ExpectCallDestructor(iamock);
	iamock->a();
	delete iamock;
	try
	{
		iamock->a();
	}
	catch(HippoMocks::ZombieMockException &)
	{
		exceptionCaught = true;
	}
	EXPECT_TRUE(exceptionCaught);
}

