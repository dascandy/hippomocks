#include "yaffut.h"
#include "hippomocks.h"

class IL { 
public:
	void f() {}
	virtual void g() {}
};

#ifndef __GCC__
template <int s>
bool is_virtual_func(unsigned char *func) {
	if (*func == 0xE9)
	{
		return is_virtual_func<0>(func + 5 + *(unsigned int *)(func+1));
	}
	else
	{
		func += 2;
		switch(*(unsigned short *)func)
		{
		case 0x20ff:
		case 0x60ff:
		case 0xA0ff: return true;
		default: return false;
		}
	}
}
#endif

template <typename T>
bool is_virtual(T t)
{
	unsigned int target = *horrible_cast<unsigned int *>(&t);
#ifdef __GCC__
	// simple implementation
	return target & 1;
#else
	return is_virtual_func<0>((unsigned char *)target);
#endif
}

FUNC(testVirtualIsVirtual) 
{
	CHECK(is_virtual(&IL::g));
	CHECK(!is_virtual(&IL::f));
}