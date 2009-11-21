#include "hippomocks.h"
#include "yaffut.h"

class IZombie { 
public:
	virtual ~IZombie() {}
	virtual void a() = 0;
};

FUNC (checkZombieCallsAreReported)
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
	catch(ZombieMockException &)
	{
		exceptionCaught = true;
	}
	CHECK(exceptionCaught);
}

