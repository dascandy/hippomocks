#include "hippomocks.h"

class IA { 
public:
	virtual ~IA() {}
	virtual void f() {}
	virtual void g() = 0;
};

int main() {
	// check base case
	{
		MockRepository mocks;
		IA *iamock = mocks.newMock<IA>();
		iamock->f();
		iamock->g();
		mocks.ReplayAll();
		iamock->f();
		iamock->g();
		mocks.VerifyAll();
	}

	// check that all calls need to be done
	bool exceptionCaught = false;
	try {
		{
			MockRepository mocks;
			IA *iamock = mocks.newMock<IA>();
			iamock->f();
			iamock->g();
			mocks.ReplayAll();
			iamock->f();
			mocks.VerifyAll();
		}
	} catch (ExpectationException &) {
		exceptionCaught = true;
	}
	if (!exceptionCaught) throw std::exception();

	// check that no more calls can be done
	{
		MockRepository mocks;
		IA *iamock = mocks.newMock<IA>();
		iamock->f();
		iamock->g();
		mocks.ReplayAll();
		iamock->f();
		iamock->g();
		bool exceptionCaught = true;
		try {
			iamock->f();
		} catch (ExpectationException &) {
			exceptionCaught = true;
		}
		if (!exceptionCaught) throw std::exception();
	}

	// check that they have to be in order
	{
		MockRepository mocks;
		IA *iamock = mocks.newMock<IA>();
		iamock->f();
		iamock->g();
		mocks.ReplayAll();
		bool exceptionCaught = true;
		try {
			iamock->g();
		} catch (ExpectationException &) {
			exceptionCaught = true;
		}
		if (!exceptionCaught) throw std::exception();
	}
	return 0;
}

