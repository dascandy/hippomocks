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
		switch(*(unsigned int *)func)
		{ // mov ecx, this; jump [eax + v/Ib/Iw]
		case 0x20ff018b:
		case 0x60ff018b:
		case 0xA0ff018b: return true;
		default: return false;
		}
	}
}
#endif

template <typename T>
bool is_virtual(T t)
{
	union {
		T t;
		unsigned long value;
	} conv;
	conv.t = t;
#ifdef __GNUG__
	// simple implementation
	return conv.value & 1;
#else
	return is_virtual_func<0>((unsigned char *)conv.value);
#endif
}

FUNC(testVirtualIsVirtual) 
{
	CHECK(is_virtual(&IL::g));
	CHECK(!is_virtual(&IL::f));
}
