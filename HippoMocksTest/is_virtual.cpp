#include "yaffut.h"
#include <iostream>
#include "hippomocks.h"

class IL { 
public:
	void f() { std::cout << "0" << std::endl; }
	virtual void g() { std::cout << "1" << std::endl; }
	virtual void h() { std::cout << "2" << std::endl; }
};

class SecondBase
{
public:
	virtual void k() { std::cout << "3" << std::endl; }
	int x;
};

class ThirdBase
{
public:
	virtual void l() { std::cout << "4" << std::endl; }
	virtual void m() { std::cout << "4" << std::endl; }
};

class ILD : public IL, public SecondBase, public ThirdBase
{
};

FUNC(checkNonVirtual) 
{
	CHECK(virtual_index(&ILD::f).first == -1);
}

FUNC(checkFirstVirtual) 
{
	CHECK(virtual_index(&ILD::g).first == 0);
	CHECK(virtual_index(&ILD::g).second == 0);
}

FUNC(checkSecondVirtual) 
{
	CHECK(virtual_index(&ILD::h).first == 0);
	CHECK(virtual_index(&ILD::h).second == 1);
}

FUNC(checkSecondBaseFirstVirtual) 
{
	CHECK(virtual_index((void (ILD::*)())&ILD::k).first == 1);
	CHECK(virtual_index((void (ILD::*)())&ILD::k).second == 0);
}

FUNC(checkThirdBaseSecondVirtualAfterInt) 
{
	CHECK(virtual_index((void (ILD::*)())&ILD::m).first == 3);
	CHECK(virtual_index((void (ILD::*)())&ILD::m).second == 1);
}

FUNC(checkPointerConversionIsOk) 
{
	void (ThirdBase::*f)() = &ThirdBase::m;
	CHECK(virtual_index((void (ILD::*)())f).first == 3);
	CHECK(virtual_index((void (ILD::*)())f).second == 1);
}

