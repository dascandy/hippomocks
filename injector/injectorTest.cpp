#include "yaffut.h"
#include "injector.h"

// "volatile" prevents the compiler from eliminating the "CHECK(!hit)" code.
static volatile bool hit = false;

int ReplaceHelloWorld(int i);

static void* somewhere = ReplaceHelloWorld;

// The compiler will inline this function without this pragma.
#pragma auto_inline(off)
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
	r = HelloWorld(2);
	CHECK(r != 42);
	CHECK(!hit);
}
