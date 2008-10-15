#include "yaffut.h"
#include "injector.h"

static bool hit = false;

int HelloWorld(int i)
{
	// mystic code...
	for (int j=i; j; --j)
	{
		i++;
	}
	return i;
}

int ReplaceHelloWorld(int i)
{
	hit = true;
	return 42;
}

// This will get further in release mode, because debug mode functions always
// start with a jmp (which is actually easier to detour hahaha)
FUNC (checkInjection)
{
	hit = false;
	void* result = inject(HelloWorld, ReplaceHelloWorld);
	// pass....
	CHECK(result != NULL);
	int r = HelloWorld(1);
	CHECK(r == 42);
	CHECK(hit);
	uninject(result);
	hit = false;
	r = HelloWorld(1);
	CHECK(r != 42);
	CHECK(!hit);

}
