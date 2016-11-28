#include "hippomocks.h"
#include "Framework.h"

class IDynamic {
public:
  virtual void a() {}
};

class ISubclassA : public virtual IDynamic {
public:
  virtual void b() {}
};

class ISubclassB : public virtual IDynamic {
public:
  virtual void c() {}
};

class IDiamondChild : public ISubclassA, public ISubclassB {
public:
  virtual void d() {}
};

// MSVC implementation incomplete
#ifndef _MSC_VER
TEST (checkCanDynamicCastAround)
{
	MockRepository mocks;
  IDiamondChild *child = mocks.Mock<IDiamondChild>();
  ISubclassA* a = child;
  ISubclassB* b = dynamic_cast<ISubclassB*>(a);
  ISubclassB* bC = child;
  CHECK(bC == b); // dynamic_cast worked and got the right offsets
  CHECK((void*)b != (void*)a);
}
#endif


