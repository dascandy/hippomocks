// HippoMocks, a library for using mocks in unit testing of C++ code.
// Copyright (C) 2008, Bas van Tiel, Christian Rexwinkel, Mike Looijmans,
// Peter Bindels
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can also retrieve it from http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef HIPPOMOCKS_H
#define HIPPOMOCKS_H

// If you want to put all HippoMocks symbols into the global namespace, use the define below.
//#define NO_HIPPOMOCKS_NAMESPACE

// The DEFAULT_AUTOEXPECT is an option that determines whether tests are, by default, under- or
// overspecified. Auto-expect, by function, adds an expectation to the current ExpectCall that
// it will happen after the previous ExpectCall. For many people this is an intuitive and logical
// thing when writing a C++ program. Usually, this makes your test workable but overspecified.
// Overspecified means that your test will fail on working code that does things in a different
// order. The alternative, underspecified, allows code to pass your test that does things in a
// different order, where that different order should be considered wrong. Consider reading a
// file, where it needs to be first opened, then read and then closed.
//
// The default is to make tests overspecified. At least it prevents faulty code from passing
// unit tests. To locally disable (or enable) this behaviour, set the boolean autoExpect on your
// MockRepository to false (or true). To globally override, redefine DEFAULT_AUTOEXPECT to false.
#ifndef DEFAULT_AUTOEXPECT
#define DEFAULT_AUTOEXPECT true
#endif

#ifdef NO_HIPPOMOCKS_NAMESPACE
#define HM_NS
#else
#define HM_NS HippoMocks::
#endif

#ifdef _MSC_VER
#ifdef _WIN64
#define WINCALL
#else
#define WINCALL __stdcall
#endif
#endif
#ifndef DEBUGBREAK
#ifdef _MSC_VER
extern "C" __declspec(dllimport) int WINCALL IsDebuggerPresent();
extern "C" __declspec(dllimport) void WINCALL DebugBreak();
#define DEBUGBREAK(e) if (IsDebuggerPresent()) DebugBreak(); else (void)0
#else
#define DEBUGBREAK(e)
#endif
#endif

#ifndef DONTCARE_NAME
#define DONTCARE_NAME _
#endif

#ifndef VIRT_FUNC_LIMIT
#define VIRT_FUNC_LIMIT 1024
#elif VIRT_FUNC_LIMIT > 1024
#error Adjust the code to support more than 1024 virtual functions before setting the VIRT_FUNC_LIMIT above 1024
#endif

#ifdef __GNUC__
#define EXTRA_DESTRUCTOR
#endif

#ifdef __EDG__
#define FUNCTION_BASE 3
#define FUNCTION_STRIDE 2
#else
#define FUNCTION_BASE 0
#define FUNCTION_STRIDE 1
#endif

#if defined(_M_IX86) || defined(__i386__) || defined(i386) || defined(_X86_) || defined(__THW_INTEL) ||  defined(__x86_64__) || defined(_M_X64)
#define SOME_X86
#elif defined(arm) || defined(__arm__) || defined(ARM) || defined(_ARM_) || defined(__aarch64__)
#define SOME_ARM
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define CMOCK_FUNC_PLATFORMIS64BIT
#endif

#ifdef SOME_X86
#if defined(_MSC_VER) && (defined(_WIN32) || defined(_WIN64))
#define _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
#elif defined(__linux__) && defined(__GNUC__)
#define _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
#elif defined(__APPLE__)
#define _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
#endif
#elif defined(SOME_ARM) && defined(__GNUC__)
#define _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT

// This clear-cache is *required*. The tests will fail if you remove it.
extern "C" void __clear_cache(char *beg, char *end);
#endif

#include <cstdio>
#include <list>
#include <map>
#include <memory>
#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <limits>
#include <functional>

#ifdef _MSC_VER
// these warnings are pointless and huge, and will confuse new users.
#pragma warning(push)
// If you can't generate an assignment operator the least you can do is shut up.
#pragma warning(disable: 4512)
// Alignment not right in a union?
#pragma warning(disable: 4121)
// No deprecated warnings on functions that really aren't deprecated at all.
#pragma warning(disable: 4996)
#endif

#ifndef NO_HIPPOMOCKS_NAMESPACE
namespace HippoMocks
{
#endif

#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
#include <memory.h>

#if defined(_WIN32)
#ifndef NO_HIPPOMOCKS_NAMESPACE
}
#endif

// De-windows.h-ified import to avoid including that file.
#ifdef _WIN64
extern "C" __declspec(dllimport) int WINCALL VirtualProtect(void *func, unsigned long long byteCount, unsigned long flags, unsigned long *oldFlags);
#else
extern "C" __declspec(dllimport) int WINCALL VirtualProtect(void *func, unsigned long byteCount, unsigned long flags, unsigned long *oldFlags);
#endif

#ifndef PAGE_EXECUTE_READWRITE
#define PAGE_EXECUTE_READWRITE 0x40
#endif

#ifndef NO_HIPPOMOCKS_NAMESPACE
namespace HippoMocks {
#endif

class Unprotect
{
public:
  Unprotect(void *location, size_t byteCount)
  : origFunc(location)
  , byteCount(byteCount)
  {
    VirtualProtect(origFunc, byteCount, PAGE_EXECUTE_READWRITE, &oldprotect);
  }
  ~Unprotect()
  {
    unsigned long dontcare;
    VirtualProtect(origFunc, byteCount, oldprotect, &dontcare);
  }
private:
  void *origFunc;
  size_t byteCount;
  unsigned long oldprotect;
};
#else
#include <sys/mman.h>
#include <stdint.h>

class Unprotect
{
public:
  Unprotect(void *location, size_t count)
  : origFunc((intptr_t)location & (~0xFFF))
  , byteCount(count + ((intptr_t)location - origFunc))
  {
    mprotect((void *)origFunc, this->byteCount, PROT_READ|PROT_WRITE|PROT_EXEC);
  };
  ~Unprotect()
  {
    mprotect((void *)origFunc, byteCount, PROT_READ|PROT_EXEC);
  }
private:
  intptr_t origFunc;
  int byteCount;
};
#endif

typedef unsigned int e9ptrsize_t;

template <typename T, typename U>
T horrible_cast(U u)
{
	union { T t; U u; } un;
	un.u = u;
	return un.t;
}

class Replace
{
private:
  void *origFunc;
  char backupData[16]; // typical use is 5 for 32-bit and 14 for 64-bit code.
public:
  template <typename T>
  Replace(T funcptr, T replacement)
	  : origFunc(horrible_cast<void *>(funcptr))
  {
	Unprotect _allow_write(origFunc, sizeof(backupData));
	memcpy(backupData, origFunc, sizeof(backupData));
#ifdef SOME_X86
#ifdef CMOCK_FUNC_PLATFORMIS64BIT
	if (llabs((long long)origFunc - (long long)replacement) < 0x80000000LL) {
#endif
	  *(unsigned char *)origFunc = 0xE9;
	  *(e9ptrsize_t*)(horrible_cast<intptr_t>(origFunc) + 1) = (e9ptrsize_t)(horrible_cast<intptr_t>(replacement) - horrible_cast<intptr_t>(origFunc) - sizeof(e9ptrsize_t) - 1);
#ifdef CMOCK_FUNC_PLATFORMIS64BIT
	} else {
	  unsigned char *func = (unsigned char *)origFunc;
	  func[0] = 0xFF; // jmp (rip + imm32)
	  func[1] = 0x25;
	  func[2] = 0x00; // imm32 of 0, so immediately after the instruction
	  func[3] = 0x00;
	  func[4] = 0x00;
	  func[5] = 0x00;
	  *(long long*)(horrible_cast<intptr_t>(origFunc) + 6) = (long long)(horrible_cast<intptr_t>(replacement));
	}
#endif
#elif defined(SOME_ARM)
	unsigned int *rawptr = (unsigned int *)((intptr_t)(origFunc) & (~3));
	if ((intptr_t)origFunc & 1) {
	  rawptr[0] = 0x6800A001;
	  rawptr[1] = 0x46874687;
	  rawptr[2] = (intptr_t)replacement;
	} else {
	  rawptr[0] = 0xE59FF000;
	  rawptr[1] = (intptr_t)replacement;
	  rawptr[2] = (intptr_t)replacement;
	}
	__clear_cache((char *)rawptr, (char *)rawptr+16);
#endif
  }
  ~Replace()
  {
	Unprotect _allow_write(origFunc, sizeof(backupData));
	memcpy(origFunc, backupData, sizeof(backupData));
#ifdef SOME_ARM
	unsigned int *rawptr = (unsigned int *)((intptr_t)(origFunc) & (~3));
	__clear_cache((char *)rawptr, (char *)rawptr+16);
#endif
  }
};
#endif

class MockRepository;

struct
RegistrationType
{
   RegistrationType( unsigned min, unsigned max ) : minimum( min ), maximum( max ) {}
   unsigned minimum;
   unsigned maximum;
};

inline
bool operator==( RegistrationType const& rhs, RegistrationType const& lhs )
{
   return rhs.minimum == lhs.minimum && rhs.maximum == lhs.maximum;
}

const RegistrationType Any = RegistrationType((std::numeric_limits<unsigned>::min)(), (std::numeric_limits<unsigned>::max)());
const RegistrationType Never = RegistrationType((std::numeric_limits<unsigned>::min)(), (std::numeric_limits<unsigned>::min)());
const RegistrationType Once = RegistrationType( 1, 1 );

// base type
class base_mock {
public:
	void destroy() { unwriteVft(); delete this; }
	virtual ~base_mock() {}
	void *rewriteVft(void *newVf)
	{
		void *oldVf = *(void **)this;
		*(void **)this = newVf;
		return oldVf;
	}
	void reset()
	{
		unwriteVft();
		mock_reset();
	}
	virtual void mock_reset() = 0;
	void unwriteVft()
	{
		*(void **)this = (*(void ***)this)[VIRT_FUNC_LIMIT+1];
	}
};

class NullType
{
public:
	bool operator==(const NullType &) const
	{
		return true;
	}
};

class DontCare {
private:
  inline DontCare &Instance();
};
static DontCare DONTCARE_NAME;
inline DontCare &DontCare::Instance()
{
  return DONTCARE_NAME;
}

template <typename T>
struct OutParam: public DontCare
{
	explicit OutParam(T val): value(val) {}
	T value;
};

template <typename T>
OutParam<T> Out(T t) { return OutParam<T>(t); }

template <typename T, bool isPointer>
struct InParam;

template <typename T>
struct InParam<T, false>: public DontCare
{
	explicit InParam(T& val): value(val)
	{
	}
	T& value;
};

template <typename T>
struct InParam<T, true>: public DontCare
{
	explicit InParam(T*& val): value(val)
	{
	}
	T*& value;
};

template <typename T>
InParam<T, false> In(T& t) { return InParam<T, false>(t); }

template <typename T>
InParam<T, true> In(T*& t) { return InParam<T, true>(t); }

struct NotPrintable { template <typename T> NotPrintable(T const&) {} };

inline std::ostream &operator<<(std::ostream &os, NotPrintable const&)
{
	os << "???";
	return os;
}

inline std::ostream &operator<<(std::ostream &os, DontCare const&)
{
	os << "_";
	return os;
}

template <typename T>
inline std::ostream &operator<<(std::ostream &os, std::reference_wrapper<T> &ref) {
  os << "ref(" << ref.get() << ")";
  return os;
}

template <typename T>
struct printArg
{
	static inline void print(std::ostream &os, T arg, bool withComma)
	{
		if (withComma)
			os << ",";
		os << arg;
	}
};

template <>
struct printArg<NullType>
{
	static void print(std::ostream &, NullType , bool)
	{
	}
};

template <typename X>
struct no_cref { typedef X type; };

template <typename X>
struct no_cref<const X &> { typedef X type; };

template <typename A> struct with_const { typedef const A type; };
template <typename A> struct with_const<A &> { typedef const A &type; };
template <typename A> struct with_const<const A> { typedef const A type; };
template <typename A> struct with_const<const A &> { typedef const A &type; };

template <typename T> struct base_type { typedef T type; };
template <typename T> struct base_type<T&> { typedef T type; };
template <typename T> struct base_type<const T> { typedef T type; };
template <typename T> struct base_type<const T&> { typedef T type; };

template <typename T>
struct comparer
{
	static inline bool compare(typename with_const<T>::type a, typename with_const<T>::type b)
	{
		return a == b;
	}
	static inline bool compare(DontCare, typename with_const<T>::type)
	{
		return true;
	}
  template <typename U>
  static inline bool compare(const std::reference_wrapper<U> &a, typename with_const<T>::type b)
  {
    return &a.get() == &b;
  }
};

template <typename T>
struct IsOutParamType { enum { value = false }; };
template <typename T>
struct IsOutParamType<OutParam<T> > { enum { value = true }; };

template <typename T>
struct IsInParamType { enum { value = false }; };
template <typename T>
struct IsInParamType<InParam<T, true> > { enum { value = true }; };
template <typename T>
struct IsInParamType<InParam<T, false> > { enum { value = true }; };

template <typename T1, typename T2, bool Assign>
struct do_assign;

template <typename T1, typename T2>
struct do_assign<T1, T2*, true>
{
  static void assign_to(T1 outparam, T2 *refparam)
  {
	*refparam = outparam.value;
  }
  static void assign_from(T1 inparam, T2 *refparam)
  {
	inparam.value = refparam;
  }
};

template <typename T1, typename T2>
struct do_assign<T1, T2&, true>
{
  static void assign_to(T1 outparam, T2 &refparam)
  {
	refparam = outparam.value;
  }
  static void assign_from(T1 inparam, T2 &refparam)
  {
	inparam.value = refparam;
  }
};

template <typename T1, typename T2>
struct do_assign<T1, T2, false>
{
	static void assign_to(T1, T2) {}
	static void assign_from(T1, T2) {}
};

template <typename T1, typename T2>
void out_assign(T1 a, T2 b)
{
	do_assign<T1, T2, IsOutParamType<typename base_type<T1>::type>::value >::assign_to(a, b);
}

template <typename T1, typename T2>
void in_assign(T1 a, T2 b)
{
	do_assign<T1, T2, IsInParamType<typename base_type<T1>::type>::value >::assign_from(a, b);
}

template <typename T> struct no_array { typedef T type; };
template <typename T, int N> struct no_array<T[N]> { typedef T* type; };

template <typename B>
struct store_as
{
	typedef typename no_array<B>::type type;
};

template <typename B>
struct store_as<B&>
{
  typedef typename no_array<B>::type type;
};

inline std::ostream &operator<<(std::ostream &os, const MockRepository &repo);

template <int X>
class MockRepoInstanceHolder {
public:
	static MockRepository *instance;
};

template <int X>
MockRepository *MockRepoInstanceHolder<X>::instance;

#include "detail/oldtuple.h"
#include "detail/exceptions.h"
#include "detail/func_index.h"

class TypeDestructable {
public:
	virtual ~TypeDestructable() {}
};

template <typename A>
class MemberWrap : public TypeDestructable {
private:
	A *member;
public:
	MemberWrap(A *mem)
		: member(mem)
	{
		new (member) A();
	}
	~MemberWrap()
	{
		member->~A();
	}
};

// mock types
template <class T>
class mock : public base_mock
{
	typedef void (*funcptr)();
	friend class MockRepository;
	unsigned char remaining[sizeof(T)];
	void NotImplemented() {
		RAISEEXCEPTION(:: HM_NS NotImplementedException(MockRepoInstanceHolder<0>::instance));
	}
protected:
	std::map<int, void (**)()> funcTables;
	void (*notimplementedfuncs[VIRT_FUNC_LIMIT])();
public:
	bool isZombie;
	std::list<TypeDestructable *> members;
	MockRepository *repo;
	std::map<std::pair<int, int>, int> funcMap;
	mock(MockRepository *repository)
		: isZombie(false)
		, repo(repository)
	{
		for (int i = 0; i < VIRT_FUNC_LIMIT; i++)
		{
			notimplementedfuncs[i] = getNonvirtualMemberFunctionAddress<void (*)()>(&mock<T>::NotImplemented);
		}
		funcptr *funcTable = new funcptr[VIRT_FUNC_LIMIT+2];
		memcpy(funcTable, notimplementedfuncs, sizeof(funcptr) * VIRT_FUNC_LIMIT);
		((void **)funcTable)[VIRT_FUNC_LIMIT] = this;
		((void **)funcTable)[VIRT_FUNC_LIMIT+1] = *(void **)this;
		funcTables[0] = funcTable;
		*(void **)this = funcTable;
		for (unsigned int i = 1; i < sizeof(remaining) / sizeof(funcptr); i++)
		{
			((void **)this)[i] = (void *)notimplementedfuncs;
		}
	}
	~mock()
	{
		for (std::list<TypeDestructable *>::iterator i = members.begin(); i != members.end(); ++i)
		{
			delete *i;
		}
		for (std::map<int, void (**)()>::iterator i = funcTables.begin(); i != funcTables.end(); ++i)
		{
			delete [] i->second;
		}
	}
	void mock_reset()
	{
		MockRepository *repository = this->repo;
		// ugly but simple
		this->~mock<T>();
		new (this) mock<T>(repository);
	}
	mock<T> *getRealThis()
	{
		void ***base = (void ***)this;
		return (mock<T> *)((*base)[VIRT_FUNC_LIMIT]);
	}
	std::pair<int, int> translateX(int x)
	{
		for (std::map<std::pair<int, int>, int>::iterator i = funcMap.begin(); i != funcMap.end(); ++i)
		{
			if (i->second == x+1) return i->first;
		}
		return std::pair<int, int>(-1, 0);
	}
	template <int X>
	void mockedDestructor(int);
};

class ReturnValueHolder {
public:
	virtual ~ReturnValueHolder() {}
};

template <class T>
class ReturnValueWrapper : public ReturnValueHolder {
public:
  virtual T value() = 0;
};

template <class Y, class RY>
class ReturnValueWrapperCopy : public ReturnValueWrapper<Y> {
public:
	typename no_cref<Y>::type rv;
	ReturnValueWrapperCopy(RY retValue) : rv(retValue) {}
  virtual Y value() { return rv; };
};

template <class Y, class RY>
class ReturnValueWrapperCopy<Y, std::reference_wrapper<RY>> : public ReturnValueWrapper<Y> {
public:
	typename std::reference_wrapper<RY> rv;
	ReturnValueWrapperCopy(std::reference_wrapper<RY> retValue) : rv(retValue) {}
  virtual Y value() { return rv; };
};

//Call wrapping
class Call {
public:
	virtual bool matchesArgs(const base_tuple &tuple) = 0;
	virtual void assignArgs(base_tuple &tuple) = 0;
	ReturnValueHolder *retVal;
#ifndef HM_NO_EXCEPTIONS
	ExceptionHolder *eHolder;
#endif
	base_mock *mock;
	VirtualDestructable *functor;
	VirtualDestructable *matchFunctor;
	std::pair<int, int> funcIndex;
	std::list<Call *> previousCalls;
	unsigned called;
	RegistrationType expectation;
	bool satisfied;
	int lineno;
	const char *funcName;
	const char *fileName;
protected:
	Call(RegistrationType expect, base_mock *baseMock, const std::pair<int, int> &index, int X, const char *func, const char *file)
		: retVal(0),
#ifndef HM_NO_EXCEPTIONS
		eHolder(0),
#endif
		mock(baseMock),
		functor(0),
		matchFunctor(0),
		funcIndex(index),
		called( 0 ),
		expectation(expect),
		satisfied(false),
		lineno(X),
		funcName(func),
		fileName(file)
	{
	}
public:
	virtual const base_tuple *getArgs() const = 0;
	virtual ~Call()
	{
#ifndef HM_NO_EXCEPTIONS
		delete eHolder;
#endif
		delete functor;
		delete matchFunctor;
		delete retVal;
	}
};

std::ostream &operator<<(std::ostream &os, const Call &call);

template <typename Y,
		  typename A = NullType, typename B = NullType, typename C = NullType, typename D = NullType,
		  typename E = NullType, typename F = NullType, typename G = NullType, typename H = NullType,
		  typename I = NullType, typename J = NullType, typename K = NullType, typename L = NullType,
		  typename M = NullType, typename N = NullType, typename O = NullType, typename P = NullType>
class TCall : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI, typename CJ, typename CK, typename CL,
			  typename CM, typename CN, typename CO, typename CP>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i, const CJ & j, const CK & k, const CL & l, const CM & m, const CN & n, const CO & o, const CP & p) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,const CJ &,const CK &,const CL &,const CM &,const CN &,const CO &,const CP &>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p);
		return *this;
	}
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &Do(T function) { functor = new DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(function); return *this; }
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N, typename O, typename P>
class TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI, typename CJ, typename CK, typename CL,
			  typename CM, typename CN, typename CO, typename CP>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i, const CJ & j, const CK & k, const CL & l, const CM & m, const CN & n, const CO & o, const CP & p) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,const CJ &,const CK &,const CL &,const CM &,const CN &,const CO &,const CP &>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p);
		return *this;
	}
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &Do(T function) { functor = new DoWrapper<T,void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(function); return *this; }
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N, typename O>
class TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI, typename CJ, typename CK, typename CL,
			  typename CM, typename CN, typename CO>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i, const CJ & j, const CK & k, const CL & l, const CM & m, const CN & n, const CO & o) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,const CJ &,const CK &,const CL &,const CM &,const CN &,const CO &,NullType>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,NullType());
		return *this;
	}
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> &Do(T function) { functor = new DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType>(function); return *this; }
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N, typename O>
class TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI, typename CJ, typename CK, typename CL,
			  typename CM, typename CN, typename CO>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i, const CJ & j, const CK & k, const CL & l, const CM & m, const CN & n, const CO & o) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,const CJ &,const CK &,const CL &,const CM &,const CN &,const CO &,NullType>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,NullType());
		return *this;
	}
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> &Do(T function) { functor = new DoWrapper<T,void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType>(function); return *this; }
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N>
class TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI, typename CJ, typename CK, typename CL,
			  typename CM, typename CN>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i, const CJ & j, const CK & k, const CL & l, const CM & m, const CN & n) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,const CJ &,const CK &,const CL &,const CM &,const CN &,NullType,NullType>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,NullType(),NullType());
		return *this;
	}
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N>
class TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI, typename CJ, typename CK, typename CL,
			  typename CM, typename CN>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i, const CJ & j, const CK & k, const CL & l, const CM & m, const CN & n) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,const CJ &,const CK &,const CL &,const CM &,const CN &,NullType,NullType>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,NullType(),NullType());
		return *this;
	}
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M>
class TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI, typename CJ, typename CK, typename CL,
			  typename CM>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i, const CJ & j, const CK & k, const CL & l, const CM & m) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,const CJ &,const CK &,const CL &,const CM &,NullType,NullType,NullType>(a,b,c,d,e,f,g,h,i,j,k,l,m,NullType(),NullType(),NullType());
		return *this;
	}
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M>
class TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI, typename CJ, typename CK, typename CL,
			  typename CM>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i, const CJ & j, const CK & k, const CL & l, const CM & m) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,const CJ &,const CK &,const CL &,const CM &,NullType,NullType,NullType>(a,b,c,d,e,f,g,h,i,j,k,l,m,NullType(),NullType(),NullType());
		return *this;
	}
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,void,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L>
class TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI, typename CJ, typename CK, typename CL>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i, const CJ & j, const CK & k, const CL & l) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,const CJ &,const CK &,const CL &,NullType,NullType,NullType,NullType>(a,b,c,d,e,f,g,h,i,j,k,l,NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L>
class TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI, typename CJ, typename CK, typename CL>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i, const CJ & j, const CK & k, const CL & l) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,const CJ &,const CK &,const CL &,NullType,NullType,NullType,NullType>(a,b,c,d,e,f,g,h,i,j,k,l,NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,void,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K>
class TCall<Y,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI, typename CJ, typename CK>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i, const CJ & j, const CK & k) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,const CJ &,const CK &,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,e,f,g,h,i,j,k,NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K>
class TCall<void,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI, typename CJ, typename CK>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i, const CJ & j, const CK & k) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,const CJ &,const CK &,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,e,f,g,h,i,j,k,NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,void,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J>
class TCall<Y,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI, typename CJ>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i, const CJ & j) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,const CJ &,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,e,f,g,h,i,j,NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<Y,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J>
class TCall<void,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI, typename CJ>
	TCall<void,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i, const CJ & j) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,const CJ &,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,e,f,g,h,i,j,NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<void,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,void,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I>
class TCall<Y,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI>
	TCall<Y,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,e,f,g,h,i,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<Y,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I>
class TCall<void,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH,
			  typename CI>
	TCall<void,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h, const CI & i) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,const CI &,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,e,f,g,h,i,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<void,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,void,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H>
class TCall<Y,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH>
	TCall<Y,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,e,f,g,h,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<Y,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,Y,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H>
class TCall<void,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG, typename CH>
	TCall<void,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g, const CH & h) {
		args = new copy_tuple<A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,const CH &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,e,f,g,h,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<void,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,void,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G>
class TCall<Y,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG>
	TCall<Y,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g) {
		args = new copy_tuple<A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,e,f,g,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<Y,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,Y,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<Y,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G>
class TCall<void,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF, typename CG>
	TCall<void,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f, const CG & g) {
		args = new copy_tuple<A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,const CG &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,e,f,g,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<void,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,void,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<void,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F>
class TCall<Y,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF>
	TCall<Y,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f) {
		args = new copy_tuple<A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,e,f,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<Y,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,Y,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<Y,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A, typename B, typename C, typename D,
		  typename E, typename F>
class TCall<void,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE, typename CF>
	TCall<void,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e, const CF & f) {
		args = new copy_tuple<A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,const CF &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,e,f,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<void,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,void,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<void,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E>
class TCall<Y,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE>
	TCall<Y,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e) {
		args = new copy_tuple<A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,e,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<Y,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,Y,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<Y,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A, typename B, typename C, typename D,
		  typename E>
class TCall<void,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD,
			  typename CE>
	TCall<void,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d, const CE & e) {
		args = new copy_tuple<A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,const CE &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,e,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<void,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,void,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<void,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y,
		  typename A, typename B, typename C, typename D>
class TCall<Y,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD>
	TCall<Y,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d) {
		args = new copy_tuple<A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<Y,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,Y,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<Y,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A, typename B, typename C, typename D>
class TCall<void,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC, typename CD>
	TCall<void,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c, const CD & d) {
		args = new copy_tuple<A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,const CD &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,d,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<void,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,void,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<void,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y,
		  typename A, typename B, typename C>
class TCall<Y,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC>
	TCall<Y,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c) {
		args = new copy_tuple<A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<Y,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,Y,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<Y,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A, typename B, typename C>
class TCall<void,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB, typename CC>
	TCall<void,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b, const CC & c) {
		args = new copy_tuple<A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,const CC &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,c,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<void,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,void,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<void,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y,
		  typename A, typename B>
class TCall<Y,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB>
	TCall<Y,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b) {
		args = new copy_tuple<A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<Y,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,Y,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<Y,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A, typename B>
class TCall<void,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA, typename CB>
	TCall<void,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a, const CB & b) {
		args = new copy_tuple<A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,const CB &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,b,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<void,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,void,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<void,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y,
		  typename A>
class TCall<Y,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA>
	TCall<Y,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA & a) {
		args = new copy_tuple<A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<Y,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,Y,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<Y,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <typename A>
class TCall<void,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) {
		return (!args && !matchFunctor) ||
			(args && (*args == reinterpret_cast<const ref_tuple<A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl))) ||
			(matchFunctor && (*(TupleInvocable<bool> *)(matchFunctor))(tupl));
	}
	void assignArgs(base_tuple &tupl) {
		if(args) {
			args->assign_to(static_cast<ref_tuple<A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
			args->assign_from(static_cast<ref_tuple<A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &>(tupl));
		}
	}
	template <typename CA>
	TCall<void,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &With(const CA &a) {
		args = new copy_tuple<A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								const CA &,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(a,NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
		return *this;
	}
	TCall<void,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,void,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
	template <typename T>
	TCall<void,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Match(T function) { matchFunctor = new DoWrapper<T,bool,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

template <typename Y>
class TCall<Y,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file) {
		args = new copy_tuple<NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
								NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>
								(NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
	}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &) { return true; }
	void assignArgs(base_tuple &) {}
	TCall<Y,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<Y,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,Y,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
  template <typename RY> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};
template <>
class TCall<void,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Call {
private:
	ref_comparable_assignable_tuple<NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> *args;
public:
		const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file) {
		args = new copy_tuple<NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,
							NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>
							(NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType(),NullType());
	}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &) { return true; }
	void assignArgs(base_tuple &) { }
	TCall<void,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &After(Call &call) {
		previousCalls.push_back(&call);
		return *this;
	}
	template <typename T>
	TCall<void,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> &Do(T function) { functor = new DoWrapper<T,void,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType>(function); return *this; }
#ifndef HM_NO_EXCEPTIONS
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

inline
bool satisfied( std::list<Call *> const& previousCalls )
{
   for (std::list<Call *>::const_iterator callsBefore = previousCalls.begin();
	  callsBefore != previousCalls.end(); ++callsBefore)
   {
	  if (!(*callsBefore)->satisfied)
	  {
		 return false;
	  }
   }
   return true;
}


class MockRepository {
private:
	friend inline std::ostream &operator<<(std::ostream &os, const MockRepository &repo);
	std::list<base_mock *> mocks;
	std::map<void (*)(), int> staticFuncMap;
#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
	std::list<Replace *> staticReplaces;
#endif

	std::list<Call *> neverCalls;
	std::list<Call *> expectations;
	std::list<Call *> optionals;
public:
	bool autoExpect;
private:

	void addAutoExpectTo( Call* call )
	{
		 if (autoExpect && expectations.size() > 0)
		 {
			call->previousCalls.push_back(expectations.back());
		 }
	}


	void addCall( Call* call, RegistrationType expect )
	{
	   if( expect == Never ) {
		  addAutoExpectTo( call );
		  neverCalls.push_back(call);
		}
		else if( expect.minimum == expect.maximum )
		{
		   addAutoExpectTo( call );
		   expectations.push_back(call);
		}
		else
		{
		   optionals.push_back(call);
	   }
	}

#ifndef HM_NO_EXCEPTIONS
	ExceptionHolder *latentException;
#endif
public:
#ifndef HM_NO_EXCEPTIONS
	void SetLatentException(ExceptionHolder *holder)
	{
		latentException = holder;
	}
#endif
#ifdef _MSC_VER
#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
#define OnCallFunc(func) RegisterExpect_<__COUNTER__>(&func, HM_NS Any, #func, __FILE__, __LINE__)
#define ExpectCallFunc(func) RegisterExpect_<__COUNTER__>(&func, HM_NS Once, #func, __FILE__, __LINE__)
#define NeverCallFunc(func) RegisterExpect_<__COUNTER__>(&func, HM_NS Never, #func, __FILE__, __LINE__)
#define OnCallFuncOverload(func) RegisterExpect_<__COUNTER__>(func, HM_NS Any, #func, __FILE__, __LINE__)
#define ExpectCallFuncOverload(func) RegisterExpect_<__COUNTER__>(func, HM_NS Once, #func, __FILE__, __LINE__)
#define NeverCallFuncOverload(func) RegisterExpect_<__COUNTER__>(func, HM_NS Never, #func, __FILE__, __LINE__)
#endif
#define OnCall(obj, func) RegisterExpect_<__COUNTER__>(obj, &func, HM_NS Any, #func, __FILE__, __LINE__)
#define OnCalls(obj, func, minimum) RegisterExpect_<__COUNTER__>(obj, &func, HM_NS RegistrationType(minimum,(std::numeric_limits<unsigned>::max)()), #func, __FILE__, __LINE__)
#define ExpectCall(obj, func) RegisterExpect_<__COUNTER__>(obj, &func, HM_NS Once, #func, __FILE__, __LINE__)
#define ExpectCalls(obj, func, num) RegisterExpect_<__COUNTER__>(obj, &func, HM_NS RegistrationType(num,num), #func, __FILE__, __LINE__)
#define NeverCall(obj, func) RegisterExpect_<__COUNTER__>(obj, &func, HM_NS Never, #func, __FILE__, __LINE__)
#define OnCallOverload(obj, func) RegisterExpect_<__COUNTER__>(obj, func, HM_NS Any, #func, __FILE__, __LINE__)
#define ExpectCallOverload(obj, func) RegisterExpect_<__COUNTER__>(obj, func, HM_NS Once, #func, __FILE__, __LINE__)
#define NeverCallOverload(obj, func) RegisterExpect_<__COUNTER__>(obj, func, HM_NS Never, #func, __FILE__, __LINE__)
#define OnCallDestructor(obj) RegisterExpectDestructor<__COUNTER__>(obj, HM_NS Any, __FILE__, __LINE__)
#define ExpectCallDestructor(obj) RegisterExpectDestructor<__COUNTER__>(obj, HM_NS Once, __FILE__, __LINE__)
#define NeverCallDestructor(obj) RegisterExpectDestructor<__COUNTER__>(obj, HM_NS Never, __FILE__, __LINE__)
#else
#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
#define OnCallFunc(func) RegisterExpect_<__LINE__>(&func, HM_NS Any, #func, __FILE__, __LINE__)
#define ExpectCallFunc(func) RegisterExpect_<__LINE__>(&func, HM_NS Once, #func, __FILE__, __LINE__)
#define NeverCallFunc(func) RegisterExpect_<__LINE__>(&func, HM_NS Never, #func, __FILE__, __LINE__)
#define OnCallFuncOverload(func) RegisterExpect_<__LINE__>(func, HM_NS Any, #func,  __FILE__, __LINE__)
#define ExpectCallFuncOverload(func) RegisterExpect_<__LINE__>(func, HM_NS Once, #func, __FILE__, __LINE__)
#define NeverCallFuncOverload(func) RegisterExpect_<__LINE__>(func, HM_NS Never, #func, __FILE__, __LINE__)
#endif
#define OnCall(obj, func) RegisterExpect_<__LINE__>(obj, &func, HM_NS Any, #func, __FILE__, __LINE__)
#define OnCalls(obj, func, minimum) RegisterExpect_<__LINE__>(obj, &func, HM_NS RegistrationType(minimum,(std::numeric_limits<unsigned>::max)()), #func, __FILE__, __LINE__)
#define ExpectCall(obj, func) RegisterExpect_<__LINE__>(obj, &func, HM_NS Once, #func, __FILE__, __LINE__)
#define ExpectCalls(obj, func, num) RegisterExpect_<__LINE__>(obj, &func, HM_NS RegistrationType(num,num), #func, __FILE__, __LINE__)
#define NeverCall(obj, func) RegisterExpect_<__LINE__>(obj, &func, HM_NS Never, #func, __FILE__, __LINE__)
#define OnCallOverload(obj, func) RegisterExpect_<__LINE__>(obj, func, HM_NS Any, #func, __FILE__, __LINE__)
#define ExpectCallOverload(obj, func) RegisterExpect_<__LINE__>(obj, func, HM_NS Once, #func, __FILE__, __LINE__)
#define NeverCallOverload(obj, func) RegisterExpect_<__LINE__>(obj, func, HM_NS Never, #func, __FILE__, __LINE__)
#define OnCallDestructor(obj) RegisterExpectDestructor<__LINE__>(obj, HM_NS Any, __FILE__, __LINE__)
#define ExpectCallDestructor(obj) RegisterExpectDestructor<__LINE__>(obj, HM_NS Once, __FILE__, __LINE__)
#define NeverCallDestructor(obj) RegisterExpectDestructor<__LINE__>(obj, HM_NS Never, __FILE__, __LINE__)
#endif
	template <typename A, class B, typename C>
	void Member(A *mck, C B::*member)
	{
		C A::*realMember = (C A::*)member;
		C *realRealMember = &(mck->*realMember);
		mock<A> *realMock = (mock<A> *)mck;
		realMock->members.push_back(new MemberWrap<C>(realRealMember));
	}
  template <int X, typename Z2>
	TCall<void> &RegisterExpectDestructor(Z2 *mck, RegistrationType expect, const char *fileName, unsigned long lineNo);

#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
	template <int X, typename Y>
	TCall<Y> &RegisterExpect_(Y (*func)(), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y, typename A>
	TCall<Y,A> &RegisterExpect_(Y (*func)(A), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B>
	TCall<Y,A,B> &RegisterExpect_(Y (*func)(A,B), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C>
	TCall<Y,A,B,C> &RegisterExpect_(Y (*func)(A,B,C), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D>
	TCall<Y,A,B,C,D> &RegisterExpect_(Y (*func)(A,B,C,D), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E>
	TCall<Y,A,B,C,D,E> &RegisterExpect_(Y (*func)(A,B,C,D,E), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F>
	TCall<Y,A,B,C,D,E,F> &RegisterExpect_(Y (*func)(A,B,C,D,E,F), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G>
	TCall<Y,A,B,C,D,E,F,G> &RegisterExpect_(Y (*func)(A,B,C,D,E,F,G), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H>
	TCall<Y,A,B,C,D,E,F,G,H> &RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I>
	TCall<Y,A,B,C,D,E,F,G,H,I> &RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J>
	TCall<Y,A,B,C,D,E,F,G,H,I,J> &RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I,J), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I,J,K), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I,J,K,L), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I,J,K,L,M), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O, typename P>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);

#if defined(_MSC_VER) && !defined(_WIN64)
	template <int X, typename Y>
	TCall<Y> &RegisterExpect_(Y (__stdcall *func)(), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y, typename A>
	TCall<Y,A> &RegisterExpect_(Y (__stdcall *func)(A), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B>
	TCall<Y,A,B> &RegisterExpect_(Y (__stdcall *func)(A,B), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C>
	TCall<Y,A,B,C> &RegisterExpect_(Y (__stdcall *func)(A,B,C), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D>
	TCall<Y,A,B,C,D> &RegisterExpect_(Y (__stdcall *func)(A,B,C,D), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E>
	TCall<Y,A,B,C,D,E> &RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F>
	TCall<Y,A,B,C,D,E,F> &RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G>
	TCall<Y,A,B,C,D,E,F,G> &RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H>
	TCall<Y,A,B,C,D,E,F,G,H> &RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I>
	TCall<Y,A,B,C,D,E,F,G,H,I> &RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J>
	TCall<Y,A,B,C,D,E,F,G,H,I,J> &RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I,J), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I,J,K), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I,J,K,L), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I,J,K,L,M), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Y,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O, typename P>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
#endif
#endif

	template <int X, typename Z2, typename Y, typename Z>
	TCall<Y> &RegisterExpect_(Z2 *mck, Y (Z::*func)(), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z, typename A>
	TCall<Y,A> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B>
	TCall<Y,A,B> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C>
	TCall<Y,A,B,C> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D>
	TCall<Y,A,B,C,D> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E>
	TCall<Y,A,B,C,D,E> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F>
	TCall<Y,A,B,C,D,E,F> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G>
	TCall<Y,A,B,C,D,E,F,G> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H>
	TCall<Y,A,B,C,D,E,F,G,H> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I>
	TCall<Y,A,B,C,D,E,F,G,H,I> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J>
	TCall<Y,A,B,C,D,E,F,G,H,I,J> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O, typename P>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);

	//GCC 3.x doesn't seem to understand overloading on const or non-const member function. Volatile appears to work though.
#if !defined(__GNUC__) || __GNUC__ > 3
	template <int X, typename Z2, typename Y, typename Z>
	TCall<Y> &RegisterExpect_(Z2 *mck, Y (Z::*func)() volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)())(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z, typename A>
	TCall<Y,A> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B>
	TCall<Y,A,B> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C>
	TCall<Y,A,B,C> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D>
	TCall<Y,A,B,C,D> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E>
	TCall<Y,A,B,C,D,E> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F>
	TCall<Y,A,B,C,D,E,F> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G>
	TCall<Y,A,B,C,D,E,F,G> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H>
	TCall<Y,A,B,C,D,E,F,G,H> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I>
	TCall<Y,A,B,C,D,E,F,G,H,I> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J>
	TCall<Y,A,B,C,D,E,F,G,H,I,J> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O, typename P>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P))(func), expect, functionName ,fileName, lineNo); }

	template <int X, typename Z2, typename Y, typename Z>
	TCall<Y> &RegisterExpect_(Z2 *mck, Y (Z::*func)() const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)())(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z, typename A>
	TCall<Y,A> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B>
	TCall<Y,A,B> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C>
	TCall<Y,A,B,C> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D>
	TCall<Y,A,B,C,D> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E>
	TCall<Y,A,B,C,D,E> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F>
	TCall<Y,A,B,C,D,E,F> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G>
	TCall<Y,A,B,C,D,E,F,G> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H>
	TCall<Y,A,B,C,D,E,F,G,H> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I>
	TCall<Y,A,B,C,D,E,F,G,H,I> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J>
	TCall<Y,A,B,C,D,E,F,G,H,I,J> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O, typename P>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P))(func), expect, functionName ,fileName, lineNo); }

	template <int X, typename Z2, typename Y, typename Z>
	TCall<Y> &RegisterExpect_(Z2 *mck, Y (Z::*func)() const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
	{ return RegisterExpect_<X>(mck, (Y(Z::*)())(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z, typename A>
	TCall<Y,A> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
	{ return RegisterExpect_<X>(mck, (Y(Z::*)(A))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B>
	TCall<Y,A,B> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
	{ return RegisterExpect_<X>(mck, (Y(Z::*)(A,B))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C>
	TCall<Y,A,B,C> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D>
	TCall<Y,A,B,C,D> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E>
	TCall<Y,A,B,C,D,E> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F>
	TCall<Y,A,B,C,D,E,F> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G>
	TCall<Y,A,B,C,D,E,F,G> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H>
	TCall<Y,A,B,C,D,E,F,G,H> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I>
	TCall<Y,A,B,C,D,E,F,G,H,I> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J>
	TCall<Y,A,B,C,D,E,F,G,H,I,J> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O, typename P>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P))(func), expect, functionName ,fileName, lineNo); }
#endif

#if defined(_MSC_VER) && !defined(_WIN64)
	// COM only support - you can duplicate this for cdecl and fastcall if you want to, but those are not as common as COM.
	template <int X, typename Z2, typename Y, typename Z>
	TCall<Y> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::* func)(), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z, typename A>
	TCall<Y,A> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B>
	TCall<Y,A,B> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C>
	TCall<Y,A,B,C> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D>
	TCall<Y,A,B,C,D> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E>
	TCall<Y,A,B,C,D,E> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F>
	TCall<Y,A,B,C,D,E,F> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G>
	TCall<Y,A,B,C,D,E,F,G> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H>
	TCall<Y,A,B,C,D,E,F,G,H> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I>
	TCall<Y,A,B,C,D,E,F,G,H,I> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J>
	TCall<Y,A,B,C,D,E,F,G,H,I,J> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O, typename P>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);

	template <int X, typename Z2, typename Y, typename Z>
	TCall<Y> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)() volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)())(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z, typename A>
	TCall<Y,A> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B>
	TCall<Y,A,B> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C>
	TCall<Y,A,B,C> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D>
	TCall<Y,A,B,C,D> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E>
	TCall<Y,A,B,C,D,E> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F>
	TCall<Y,A,B,C,D,E,F> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G>
	TCall<Y,A,B,C,D,E,F,G> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H>
	TCall<Y,A,B,C,D,E,F,G,H> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I>
	TCall<Y,A,B,C,D,E,F,G,H,I> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J>
	TCall<Y,A,B,C,D,E,F,G,H,I,J> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K,L))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O, typename P>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P))(func), expect, functionName ,fileName, lineNo); }

	template <int X, typename Z2, typename Y, typename Z>
	TCall<Y> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)() const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)())(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z, typename A>
	TCall<Y,A> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B>
	TCall<Y,A,B> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C>
	TCall<Y,A,B,C> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D>
	TCall<Y,A,B,C,D> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E>
	TCall<Y,A,B,C,D,E> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F>
	TCall<Y,A,B,C,D,E,F> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G>
	TCall<Y,A,B,C,D,E,F,G> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H>
	TCall<Y,A,B,C,D,E,F,G,H> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I>
	TCall<Y,A,B,C,D,E,F,G,H,I> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J>
	TCall<Y,A,B,C,D,E,F,G,H,I,J> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K,L))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O, typename P>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P))(func), expect, functionName ,fileName, lineNo); }

	template <int X, typename Z2, typename Y, typename Z>
	TCall<Y> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)() const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)())(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z, typename A>
	TCall<Y,A> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B>
	TCall<Y,A,B> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C>
	TCall<Y,A,B,C> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D>
	TCall<Y,A,B,C,D> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E>
	TCall<Y,A,B,C,D,E> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F>
	TCall<Y,A,B,C,D,E,F> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G>
	TCall<Y,A,B,C,D,E,F,G> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H>
	TCall<Y,A,B,C,D,E,F,G,H> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I>
	TCall<Y,A,B,C,D,E,F,G,H,I> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J>
	TCall<Y,A,B,C,D,E,F,G,H,I,J> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K,L))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O))(func), expect, functionName ,fileName, lineNo); }
	template <int X, typename Z2, typename Y, typename Z,
			  typename A, typename B, typename C, typename D,
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O, typename P>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P))(func), expect, functionName ,fileName, lineNo); }
#endif


	template <typename Z>
	void BasicRegisterExpect(mock<Z> *zMock, int baseOffset, int funcIndex, void (base_mock::*func)(), int X);
#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
  int BasicStaticRegisterExpect(void (*func)(), void (*fp)(), int X)
  {
	if (staticFuncMap.find(func) == staticFuncMap.end())
	{
	  staticFuncMap[func] = X;
	  staticReplaces.push_back(new Replace(func, fp));
	}
	return staticFuncMap[func];
  }
#endif

  void doCall( Call* call, const base_tuple &tuple )
  {
	  ++call->called;

	  call->satisfied = call->called >= call->expectation.minimum;

	  call->assignArgs(const_cast<base_tuple &>(tuple));
	  #ifndef HM_NO_EXCEPTIONS
	  if (call->eHolder)
		 call->eHolder->rethrow();
	  #endif
  }

  void doVoidFunctor( Call* call, const base_tuple &tuple )
  {
	  if (call->functor != NULL)
		 (*(TupleInvocable<void> *)(call->functor))(tuple);
  }

  template<typename Z>
  Z doReturnFunctor( Call* call, const base_tuple &tuple )
  {
	  if (call->functor != NULL)
	  {
			if(call->retVal == NULL)
			{
			   return (*(TupleInvocable<Z> *)(call->functor))(tuple);
			}
			else
			{
			   (*(TupleInvocable<Z> *)(call->functor))(tuple);
			}
	  }

	  if (call->retVal)
			return ((ReturnValueWrapper<Z> *)call->retVal)->value();

	  RAISEEXCEPTION(NoResultSetUpException(this, call->getArgs(), call->funcName));
  }

  void doVoidCall( Call* call, const base_tuple &tuple, bool const makeLatent )
  {
	 if( call->called < call->expectation.maximum )
	 {
		 doCall( call, tuple );
		 doVoidFunctor( call, tuple );
	 }
	 else
	 {
		 doThrow( call, makeLatent );
	 }
  }

  template<typename Z>
  Z doReturnCall( Call* call, const base_tuple &tuple )
  {
	 if( call->called < call->expectation.maximum )
	 {
		 doCall( call, tuple );
		 return doReturnFunctor<Z>( call, tuple );
	 }
	  RAISEEXCEPTION(ExpectationException(this, call->getArgs(), call->funcName));
  }


  void doThrow( Call * call, bool const makeLatent )
  {
	  call->satisfied = true;

	  if (makeLatent)
	  {
	  #ifndef HM_NO_EXCEPTIONS
		 if (latentException)
			return;
	  #endif
		 RAISELATENTEXCEPTION(ExpectationException(this, call->getArgs(), call->funcName));
	  }
	  else
	  {
		 RAISEEXCEPTION(ExpectationException(this, call->getArgs(), call->funcName));
	  }
  }

  const char *funcName( base_mock *mock, std::pair<int, int> funcno )
  {
	  const char *name = NULL;
	  for (std::list<Call *>::iterator i = expectations.begin(); i != expectations.end() && !name; ++i)
	  {
		 Call *call = *i;
		 if (call->mock == mock &&
			   call->funcIndex == funcno)
		 name = call->funcName;
	  }
	  for (std::list<Call *>::iterator i = optionals.begin(); i != optionals.end() && !name; ++i)
	  {
		 Call *call = *i;
		 if (call->mock == mock &&
			   call->funcIndex == funcno)
		 name = call->funcName;
	  }
	  for (std::list<Call *>::iterator i = neverCalls.begin(); i != neverCalls.end() && !name; ++i)
	  {
		 Call *call = *i;
		 if (call->mock == mock &&
			   call->funcIndex == funcno)
			   name = call->funcName;
	  }
	  return name;
  }

  bool
  matches( Call *call, base_mock *mock, std::pair<int, int> funcno, const base_tuple &tuple )
  {
   return call->mock == mock &&
		  call->funcIndex == funcno &&
		  call->matchesArgs(tuple) &&
		  satisfied( call->previousCalls );

 }

   bool
   isUnsatisfied( Call *call, base_mock *mock, std::pair<int, int> funcno, const base_tuple &tuple )
   {
	  return ! call->satisfied && matches( call, mock, funcno, tuple );
   }

	template <typename Z>
	Z DoExpectation(base_mock *mock, std::pair<int, int> funcno, const base_tuple &tuple);

	void DoVoidExpectation(base_mock *mock, std::pair<int, int> funcno, const base_tuple &tuple, bool makeLatent = false)
	{
		for (std::list<Call *>::reverse_iterator i = neverCalls.rbegin(); i != neverCalls.rend(); ++i)
		{
			Call *call = *i;
			if ( matches( call, mock, funcno, tuple ) )
			{
			   doThrow( call, makeLatent );
			   return;
			}
		}
	   for (std::list<Call *>::reverse_iterator i = expectations.rbegin(); i != expectations.rend(); ++i)
		{
			Call *call = *i;
			if ( isUnsatisfied( call, mock, funcno, tuple ) )
			{
				doVoidCall( call, tuple, makeLatent );
				return;
			}
		}
		for (std::list<Call *>::reverse_iterator i = optionals.rbegin(); i != optionals.rend(); ++i)
		{
			Call *call = *i;
			if ( matches( call, mock, funcno, tuple ) )
			{
			   doVoidCall( call, tuple, makeLatent );
			   return;
			}
		}

		if (makeLatent)
		{
#ifndef HM_NO_EXCEPTIONS
			if (latentException)
				return;
#endif
			RAISELATENTEXCEPTION(ExpectationException(this, &tuple, funcName(mock, funcno)));
		}
		else
		{
			RAISEEXCEPTION(ExpectationException(this, &tuple, funcName(mock, funcno)));
		}
	}
	MockRepository()
		: autoExpect(DEFAULT_AUTOEXPECT)
#ifndef HM_NO_EXCEPTIONS
		, latentException(0)
#endif
	{
		MockRepoInstanceHolder<0>::instance = this;
	}
	~MockRepository()
  // If we're on a recent enough compiler that's not VS2012 (as it doesn't have noexcept)
#if __cplusplus > 199711L || (defined(_MSC_VER) && _MSC_VER > 1700)
noexcept(false)
#endif
	{
		MockRepoInstanceHolder<0>::instance = 0;
#ifndef HM_NO_EXCEPTIONS
		if (!std::uncaught_exception())
		{
			try
			{
#endif
				VerifyAll();
#ifndef HM_NO_EXCEPTIONS
			}
			catch(...)
			{
				delete latentException;
				reset();
				for (std::list<base_mock *>::iterator i = mocks.begin(); i != mocks.end(); i++)
				{
					(*i)->destroy();
				}
#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
				for (std::list<Replace *>::iterator i = staticReplaces.begin(); i != staticReplaces.end(); i++)
				{
					delete *i;
				}
#endif
				throw;
			}
		}
		if (latentException)
		{
			try
			{
				latentException->rethrow();
			}
			catch(BASE_EXCEPTION e)
			{
				printf("Latent exception masked!\nException:\n%s\n", e.what());
			}
			delete latentException;
		}
#endif
		reset();
		for (std::list<base_mock *>::iterator i = mocks.begin(); i != mocks.end(); i++)
		{
			(*i)->destroy();
		}
#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
		for (std::list<Replace *>::iterator i = staticReplaces.begin(); i != staticReplaces.end(); i++)
		{
			delete *i;
		}
#endif
	}
	void reset()
	{
	   for (std::list<Call *>::iterator i = expectations.begin(); i != expectations.end(); i++)
		{
			delete *i;
		}
		expectations.clear();
		for (std::list<Call *>::iterator i = neverCalls.begin(); i != neverCalls.end(); i++)
		{
			delete *i;
		}
		neverCalls.clear();
		for (std::list<Call *>::iterator i = optionals.begin(); i != optionals.end(); i++)
		{
			delete *i;
		}
		optionals.clear();
		for (std::list<base_mock *>::iterator i = mocks.begin(); i != mocks.end(); i++)
		{
			(*i)->reset();
		}
	}

	void VerifyAll()
	{
#ifndef HM_NO_EXCEPTIONS
		if (latentException)
			latentException->rethrow();
#endif

		for (std::list<Call *>::iterator i = expectations.begin(); i != expectations.end(); i++)
		{
			if (!(*i)->satisfied) {
				RAISEEXCEPTION(CallMissingException(this));
			}
		}
	}
	void VerifyPartial(base_mock *obj)
	{
#ifndef HM_NO_EXCEPTIONS
		if (latentException)
			return;
#endif
		for (std::list<Call *>::iterator i = expectations.begin(); i != expectations.end(); i++)
		{
			if ((*i)->mock == (base_mock *)obj &&
				!(*i)->satisfied )
			{
				RAISELATENTEXCEPTION(CallMissingException(this));
			}
		}
	}
	template <typename base>
	base *Mock();
};

// mock function providers
template <typename Z, typename Y>
class mockFuncs : public mock<Z> {
private:
	mockFuncs();
public:
	template <int X>
	Y expectation0()
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<>());
	}
	template <int X, typename A>
	Y expectation1(A a)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A>(a));
	}
	template <int X, typename A, typename B>
	Y expectation2(A a, B b)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A,B>(a,b));
	}
	template <int X, typename A, typename B, typename C>
	Y expectation3(A a, B b, C c)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A,B,C>(a,b,c));
	}
	template <int X, typename A, typename B, typename C, typename D>
	Y expectation4(A a, B b, C c, D d)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A,B,C,D>(a,b,c,d));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E>
	Y expectation5(A a, B b, C c, D d, E e)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E>(a,b,c,d,e));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F>
	Y expectation6(A a, B b, C c, D d, E e, F f)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F>(a,b,c,d,e,f));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G>
	Y expectation7(A a, B b, C c, D d, E e, F f, G g)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G>(a,b,c,d,e,f,g));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
	Y expectation8(A a, B b, C c, D d, E e, F f, G g, H h)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H>(a,b,c,d,e,f,g,h));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I>
	Y expectation9(A a, B b, C c, D d, E e, F f, G g, H h, I i)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I>(a,b,c,d,e,f,g,h,i));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
	Y expectation10(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J>(a,b,c,d,e,f,g,h,i,j));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K>
	Y expectation11(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K>(a,b,c,d,e,f,g,h,i,j,k));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L>
	Y expectation12(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L>(a,b,c,d,e,f,g,h,i,j,k,l));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M>
	Y expectation13(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M>(a,b,c,d,e,f,g,h,i,j,k,l,m));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N>
	Y expectation14(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N>(a,b,c,d,e,f,g,h,i,j,k,l,m,n));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O>
	Y expectation15(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O, typename P>
	Y expectation16(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p));
	}

	template <int X>
	static Y static_expectation0()
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<>());
	}
	template <int X, typename A>
	static Y static_expectation1(A a)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A>(a));
	}
	template <int X, typename A, typename B>
	static Y static_expectation2(A a, B b)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B>(a,b));
	}
	template <int X, typename A, typename B, typename C>
	static Y static_expectation3(A a, B b, C c)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C>(a,b,c));
	}
	template <int X, typename A, typename B, typename C, typename D>
	static Y static_expectation4(A a, B b, C c, D d)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D>(a,b,c,d));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E>
	static Y static_expectation5(A a, B b, C c, D d, E e)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E>(a,b,c,d,e));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F>
	static Y static_expectation6(A a, B b, C c, D d, E e, F f)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F>(a,b,c,d,e,f));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G>
	static Y static_expectation7(A a, B b, C c, D d, E e, F f, G g)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G>(a,b,c,d,e,f,g));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
	static Y static_expectation8(A a, B b, C c, D d, E e, F f, G g, H h)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H>(a,b,c,d,e,f,g,h));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I>
	static Y static_expectation9(A a, B b, C c, D d, E e, F f, G g, H h, I i)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I>(a,b,c,d,e,f,g,h,i));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
	static Y static_expectation10(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J>(a,b,c,d,e,f,g,h,i,j));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K>
	static Y static_expectation11(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K>(a,b,c,d,e,f,g,h,i,j,k));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L>
	static Y static_expectation12(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L>(a,b,c,d,e,f,g,h,i,j,k,l));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M>
	static Y static_expectation13(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M>(a,b,c,d,e,f,g,h,i,j,k,l,m));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N>
	static Y static_expectation14(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N>(a,b,c,d,e,f,g,h,i,j,k,l,m,n));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O>
	static Y static_expectation15(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O, typename P>
	static Y static_expectation16(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p));
	}

#ifdef _MSC_VER
	template <int X>
	Y __stdcall stdcallexpectation0()
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<>());
	}
	template <int X, typename A>
	Y __stdcall stdcallexpectation1(A a)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A>(a));
	}
	template <int X, typename A, typename B>
	Y __stdcall stdcallexpectation2(A a, B b)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A,B>(a,b));
	}
	template <int X, typename A, typename B, typename C>
	Y __stdcall stdcallexpectation3(A a, B b, C c)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A,B,C>(a,b,c));
	}
	template <int X, typename A, typename B, typename C, typename D>
	Y __stdcall stdcallexpectation4(A a, B b, C c, D d)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D>(a,b,c,d));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E>
	Y __stdcall stdcallexpectation5(A a, B b, C c, D d, E e)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E>(a,b,c,d,e));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F>
	Y __stdcall stdcallexpectation6(A a, B b, C c, D d, E e, F f)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F>(a,b,c,d,e,f));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G>
	Y __stdcall stdcallexpectation7(A a, B b, C c, D d, E e, F f, G g)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G>(a,b,c,d,e,f,g));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
	Y __stdcall stdcallexpectation8(A a, B b, C c, D d, E e, F f, G g, H h)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H>(a,b,c,d,e,f,g,h));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I>
	Y __stdcall stdcallexpectation9(A a, B b, C c, D d, E e, F f, G g, H h, I i)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I>(a,b,c,d,e,f,g,h,i));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
	Y __stdcall stdcallexpectation10(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J>(a,b,c,d,e,f,g,h,i,j));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K>
	Y __stdcall stdcallexpectation11(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K>(a,b,c,d,e,f,g,h,i,j,k));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L>
	Y __stdcall stdcallexpectation12(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L>(a,b,c,d,e,f,g,h,i,j,k,l));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M>
	Y __stdcall stdcallexpectation13(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M>(a,b,c,d,e,f,g,h,i,j,k,l,m));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N>
	Y __stdcall stdcallexpectation14(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N>(a,b,c,d,e,f,g,h,i,j,k,l,m,n));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O>
	Y __stdcall stdcallexpectation15(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O, typename P>
	Y __stdcall stdcallexpectation16(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p)
	{
		MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p));
	}
#if defined(_MSC_VER) && !defined(_WIN64)
	template <int X>
	static Y __stdcall static_stdcallexpectation0()
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<>());
	}
	template <int X, typename A>
	static Y __stdcall static_stdcallexpectation1(A a)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A>(a));
	}
	template <int X, typename A, typename B>
	static Y __stdcall static_stdcallexpectation2(A a, B b)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B>(a,b));
	}
	template <int X, typename A, typename B, typename C>
	static Y __stdcall static_stdcallexpectation3(A a, B b, C c)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C>(a,b,c));
	}
	template <int X, typename A, typename B, typename C, typename D>
	static Y __stdcall static_stdcallexpectation4(A a, B b, C c, D d)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D>(a,b,c,d));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E>
	static Y __stdcall static_stdcallexpectation5(A a, B b, C c, D d, E e)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E>(a,b,c,d,e));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F>
	static Y __stdcall static_stdcallexpectation6(A a, B b, C c, D d, E e, F f)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F>(a,b,c,d,e,f));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G>
	static Y __stdcall static_stdcallexpectation7(A a, B b, C c, D d, E e, F f, G g)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G>(a,b,c,d,e,f,g));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
	static Y __stdcall static_stdcallexpectation8(A a, B b, C c, D d, E e, F f, G g, H h)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H>(a,b,c,d,e,f,g,h));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I>
	static Y __stdcall static_stdcallexpectation9(A a, B b, C c, D d, E e, F f, G g, H h, I i)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I>(a,b,c,d,e,f,g,h,i));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
	static Y __stdcall static_stdcallexpectation10(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J>(a,b,c,d,e,f,g,h,i,j));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K>
	static Y __stdcall static_stdcallexpectation11(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K>(a,b,c,d,e,f,g,h,i,j,k));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L>
	static Y __stdcall static_stdcallexpectation12(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L>(a,b,c,d,e,f,g,h,i,j,k,l));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M>
	static Y __stdcall static_stdcallexpectation13(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M>(a,b,c,d,e,f,g,h,i,j,k,l,m));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N>
	static Y __stdcall static_stdcallexpectation14(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N>(a,b,c,d,e,f,g,h,i,j,k,l,m,n));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O>
	static Y __stdcall static_stdcallexpectation15(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O, typename P>
	static Y __stdcall static_stdcallexpectation16(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p)
	{
		return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p));
	}
#endif
#endif
};

template <typename Z>
class mockFuncs<Z, void> : public mock<Z> {
private:
		mockFuncs();
public:
	template <int X>
	void expectation0()
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<>());
	}
	template <int X, typename A>
	void expectation1(A a)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A>(a));
	}
	template <int X, typename A, typename B>
	void expectation2(A a, B b)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A,B>(a,b));
	}
	template <int X, typename A, typename B, typename C>
	void expectation3(A a, B b, C c)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A,B,C>(a,b,c));
	}
	template <int X, typename A, typename B, typename C, typename D>
	void expectation4(A a, B b, C c, D d)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A,B,C,D>(a,b,c,d));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E>
	void expectation5(A a, B b, C c, D d, E e)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E>(a,b,c,d,e));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F>
	void expectation6(A a, B b, C c, D d, E e, F f)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F>(a,b,c,d,e,f));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G>
	void expectation7(A a, B b, C c, D d, E e, F f, G g)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G>(a,b,c,d,e,f,g));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
	void expectation8(A a, B b, C c, D d, E e, F f, G g, H h)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H>(a,b,c,d,e,f,g,h));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I>
	void expectation9(A a, B b, C c, D d, E e, F f, G g, H h, I i)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I>(a,b,c,d,e,f,g,h,i));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
	void expectation10(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J>(a,b,c,d,e,f,g,h,i,j));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K>
	void expectation11(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K>(a,b,c,d,e,f,g,h,i,j,k));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L>
	void expectation12(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L>(a,b,c,d,e,f,g,h,i,j,k,l));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M>
	void expectation13(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M>(a,b,c,d,e,f,g,h,i,j,k,l,m));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N>
	void expectation14(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N>(a,b,c,d,e,f,g,h,i,j,k,l,m,n));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O>
	void expectation15(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O, typename P>
	void expectation16(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p)
	{
		mock<Z> *realMock = mock<Z>::getRealThis();
		if (realMock->isZombie)
			RAISEEXCEPTION(ZombieMockException(realMock->repo));
		MockRepository *repo = realMock->repo;
		repo->DoVoidExpectation(realMock, realMock->translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p));
	}
	template <int X>
	static void static_expectation0()
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<>());
	}
	template <int X, typename A>
	static void static_expectation1(A a)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A>(a));
	}
	template <int X, typename A, typename B>
	static void static_expectation2(A a, B b)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B>(a,b));
	}
	template <int X, typename A, typename B, typename C>
	static void static_expectation3(A a, B b, C c)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C>(a,b,c));
	}
	template <int X, typename A, typename B, typename C, typename D>
	static void static_expectation4(A a, B b, C c, D d)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D>(a,b,c,d));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E>
	static void static_expectation5(A a, B b, C c, D d, E e)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E>(a,b,c,d,e));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F>
	static void static_expectation6(A a, B b, C c, D d, E e, F f)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F>(a,b,c,d,e,f));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G>
	static void static_expectation7(A a, B b, C c, D d, E e, F f, G g)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G>(a,b,c,d,e,f,g));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
	static void static_expectation8(A a, B b, C c, D d, E e, F f, G g, H h)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H>(a,b,c,d,e,f,g,h));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I>
	static void static_expectation9(A a, B b, C c, D d, E e, F f, G g, H h, I i)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I>(a,b,c,d,e,f,g,h,i));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
	static void static_expectation10(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J>(a,b,c,d,e,f,g,h,i,j));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K>
	static void static_expectation11(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K>(a,b,c,d,e,f,g,h,i,j,k));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L>
	static void static_expectation12(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L>(a,b,c,d,e,f,g,h,i,j,k,l));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M>
	static void static_expectation13(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M>(a,b,c,d,e,f,g,h,i,j,k,l,m));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N>
	static void static_expectation14(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N>(a,b,c,d,e,f,g,h,i,j,k,l,m,n));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O>
	static void static_expectation15(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O, typename P>
	static void static_expectation16(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p)
	{
		MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p));
	}

#ifdef _MSC_VER
	template <int X>
	void __stdcall stdcallexpectation0()
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<>());
	}
	template <int X, typename A>
	void __stdcall stdcallexpectation1(A a)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A>(a));
	}
	template <int X, typename A, typename B>
	void __stdcall stdcallexpectation2(A a, B b)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A,B>(a,b));
	}
	template <int X, typename A, typename B, typename C>
	void __stdcall stdcallexpectation3(A a, B b, C c)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A,B,C>(a,b,c));
	}
	template <int X, typename A, typename B, typename C, typename D>
	void __stdcall stdcallexpectation4(A a, B b, C c, D d)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D>(a,b,c,d));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E>
	void __stdcall stdcallexpectation5(A a, B b, C c, D d, E e)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E>(a,b,c,d,e));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F>
	void __stdcall stdcallexpectation6(A a, B b, C c, D d, E e, F f)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F>(a,b,c,d,e,f));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G>
	void __stdcall stdcallexpectation7(A a, B b, C c, D d, E e, F f, G g)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G>(a,b,c,d,e,f,g));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
	void __stdcall stdcallexpectation8(A a, B b, C c, D d, E e, F f, G g, H h)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H>(a,b,c,d,e,f,g,h));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I>
	void __stdcall stdcallexpectation9(A a, B b, C c, D d, E e, F f, G g, H h, I i)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I>(a,b,c,d,e,f,g,h,i));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
	void __stdcall stdcallexpectation10(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J>(a,b,c,d,e,f,g,h,i,j));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K>
	void __stdcall stdcallexpectation11(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K>(a,b,c,d,e,f,g,h,i,j,k));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L>
	void __stdcall stdcallexpectation12(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L>(a,b,c,d,e,f,g,h,i,j,k,l));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M>
	void __stdcall stdcallexpectation13(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M>(a,b,c,d,e,f,g,h,i,j,k,l,m));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N>
	void __stdcall stdcallexpectation14(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N>(a,b,c,d,e,f,g,h,i,j,k,l,m,n));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O>
	void __stdcall stdcallexpectation15(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O, typename P>
	void __stdcall stdcallexpectation16(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p)
	{
		MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p));
	}
#if defined(_MSC_VER) && !defined(_WIN64)
	template <int X>
	static void __stdcall static_stdcallexpectation0()
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<>());
	}
	template <int X, typename A>
	static void __stdcall static_stdcallexpectation1(A a)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A>(a));
	}
	template <int X, typename A, typename B>
	static void __stdcall static_stdcallexpectation2(A a, B b)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B>(a,b));
	}
	template <int X, typename A, typename B, typename C>
	static void __stdcall static_stdcallexpectation3(A a, B b, C c)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C>(a,b,c));
	}
	template <int X, typename A, typename B, typename C, typename D>
	static void __stdcall static_stdcallexpectation4(A a, B b, C c, D d)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D>(a,b,c,d));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E>
	static void __stdcall static_stdcallexpectation5(A a, B b, C c, D d, E e)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E>(a,b,c,d,e));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F>
	static void __stdcall static_stdcallexpectation6(A a, B b, C c, D d, E e, F f)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F>(a,b,c,d,e,f));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G>
	static void __stdcall static_stdcallexpectation7(A a, B b, C c, D d, E e, F f, G g)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G>(a,b,c,d,e,f,g));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
	static void __stdcall static_stdcallexpectation8(A a, B b, C c, D d, E e, F f, G g, H h)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H>(a,b,c,d,e,f,g,h));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I>
	static void __stdcall static_stdcallexpectation9(A a, B b, C c, D d, E e, F f, G g, H h, I i)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I>(a,b,c,d,e,f,g,h,i));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
	static void __stdcall static_stdcallexpectation10(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J>(a,b,c,d,e,f,g,h,i,j));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K>
	static void __stdcall static_stdcallexpectation11(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K>(a,b,c,d,e,f,g,h,i,j,k));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L>
	static void __stdcall static_stdcallexpectation12(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L>(a,b,c,d,e,f,g,h,i,j,k,l));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M>
	static void __stdcall static_stdcallexpectation13(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M>(a,b,c,d,e,f,g,h,i,j,k,l,m));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N>
	static void __stdcall static_stdcallexpectation14(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N>(a,b,c,d,e,f,g,h,i,j,k,l,m,n));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O>
	static void __stdcall static_stdcallexpectation15(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O, typename P>
	static void __stdcall static_stdcallexpectation16(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p)
	{
		return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p));
	}
#endif
#endif
};

template <typename T>
template <int X>
void mock<T>::mockedDestructor(int)
{
	repo->DoVoidExpectation(this, translateX(X), ref_tuple<>(), true);
	repo->VerifyPartial(this);
	isZombie = true;
}

template <typename Z>
void MockRepository::BasicRegisterExpect(mock<Z> *zMock, int baseOffset, int funcIndex, void (base_mock::*func)(), int X)
{
	if (funcIndex > VIRT_FUNC_LIMIT) RAISEEXCEPTION(NotImplementedException(this));
	if ((unsigned int)baseOffset * sizeof(void*) + sizeof(void*)-1 > sizeof(Z)) RAISEEXCEPTION(NotImplementedException(this));
	if (zMock->funcMap.find(std::make_pair(baseOffset, funcIndex)) == zMock->funcMap.end())
	{
		if (zMock->funcTables.find(baseOffset) == zMock->funcTables.end())
		{
			typedef void (*funcptr)();
			funcptr *funcTable = new funcptr[VIRT_FUNC_LIMIT+1];
			memcpy(funcTable, zMock->notimplementedfuncs, sizeof(funcptr) * VIRT_FUNC_LIMIT);
			((void **)funcTable)[VIRT_FUNC_LIMIT] = zMock;
			zMock->funcTables[baseOffset] = funcTable;
			((void **)zMock)[baseOffset] = funcTable;
		}
		zMock->funcMap[std::make_pair(baseOffset, funcIndex)] = X+1;
		zMock->funcTables[baseOffset][funcIndex] = getNonvirtualMemberFunctionAddress<void (*)()>(func);
	}
}

template <int X, typename Z2>
TCall<void> &MockRepository::RegisterExpectDestructor(Z2 *mck, RegistrationType expect, const char *fileName, unsigned long lineNo)
{
	func_index idx;
	((Z2 *)&idx)->~Z2();
	int funcIndex = idx.lci * FUNCTION_STRIDE + FUNCTION_BASE;
	void (mock<Z2>::*member)(int);
	member = &mock<Z2>::template mockedDestructor<X>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						0, funcIndex,
						reinterpret_cast<void (base_mock::*)()>(member), X);
#ifdef EXTRA_DESTRUCTOR
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						0, funcIndex+1,
						reinterpret_cast<void (base_mock::*)()>(member), X);
#endif
	TCall<void> *call = new TCall<void>(Once, reinterpret_cast<base_mock *>(mck), std::pair<int, int>(0, funcIndex), lineNo, "destructor", fileName);
	addCall( call, expect );
	return *call;
}

#if defined(_MSC_VER) && !defined(_WIN64)
// Support for COM, see declarations
template <int X, typename Z2, typename Y, typename Z>
TCall<Y> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)())func);
	Y(__stdcall mockFuncs<Z2, Y>::*mfp)();
	mfp = &mockFuncs<Z2, Y>::template stdcallexpectation0<X>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
		funcIndex.first,
		funcIndex.second,
		reinterpret_cast<void (base_mock::*)()>(mfp), X);
	TCall<Y> *call = new TCall<Y>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);

	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z, typename A>
TCall<Y,A> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A))func);
	Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A);
	mfp = &mockFuncs<Z2, Y>::template stdcallexpectation1<X, A>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
		funcIndex.first,
		funcIndex.second,
		reinterpret_cast<void (base_mock::*)()>(mfp), X);
	TCall<Y, A> *call = new TCall<Y, A>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z,
	typename A, typename B>
TCall<Y,A,B> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
		std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A, B))func);
		Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A, B);
		mfp = &mockFuncs<Z2, Y>::template stdcallexpectation2<X, A, B>;
		BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
			funcIndex.first,
			funcIndex.second,
			reinterpret_cast<void (base_mock::*)()>(mfp), X);
		TCall<Y, A, B> *call = new TCall<Y, A, B>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
		return *call;
}
template <int X, typename Z2, typename Y, typename Z,
	typename A, typename B, typename C>
TCall<Y,A,B,C> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
		std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A, B, C))func);
		Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A, B, C);
		mfp = &mockFuncs<Z2, Y>::template stdcallexpectation3<X, A, B, C>;
		BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
			funcIndex.first,
			funcIndex.second,
			reinterpret_cast<void (base_mock::*)()>(mfp), X);
		TCall<Y, A, B, C> *call = new TCall<Y, A, B, C>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
		return *call;
}
template <int X, typename Z2, typename Y, typename Z,
	typename A, typename B, typename C, typename D>
TCall<Y,A,B,C,D> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
		std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A, B, C, D))func);
		Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A, B, C, D);
		mfp = &mockFuncs<Z2, Y>::template stdcallexpectation4<X, A, B, C, D>;
		BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
			funcIndex.first,
			funcIndex.second,
			reinterpret_cast<void (base_mock::*)()>(mfp), X);
		TCall<Y, A, B, C, D> *call = new TCall<Y, A, B, C, D>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
		return *call;
}
template <int X, typename Z2, typename Y, typename Z,
	typename A, typename B, typename C, typename D,
	typename E>
TCall<Y,A,B,C,D,E> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
		std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A, B, C, D, E))func);
		Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A, B, C, D, E);
		mfp = &mockFuncs<Z2, Y>::template stdcallexpectation5<X, A, B, C, D, E>;
		BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
			funcIndex.first,
			funcIndex.second,
			reinterpret_cast<void (base_mock::*)()>(mfp), X);
		TCall<Y, A, B, C, D, E> *call = new TCall<Y, A, B, C, D, E>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
		return *call;
}
template <int X, typename Z2, typename Y, typename Z,
	typename A, typename B, typename C, typename D,
	typename E, typename F>
TCall<Y,A,B,C,D,E,F> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
		std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A, B, C, D, E, F))func);
		Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A, B, C, D, E, F);
		mfp = &mockFuncs<Z2, Y>::template stdcallexpectation6<X, A, B, C, D, E, F>;
		BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
			funcIndex.first,
			funcIndex.second,
			reinterpret_cast<void (base_mock::*)()>(mfp), X);
		TCall<Y, A, B, C, D, E, F> *call = new TCall<Y, A, B, C, D, E, F>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
		return *call;
}
template <int X, typename Z2, typename Y, typename Z,
	typename A, typename B, typename C, typename D,
	typename E, typename F, typename G>
TCall<Y,A,B,C,D,E,F,G> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
		std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A, B, C, D, E, F, G))func);
		Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A, B, C, D, E, F, G);
		mfp = &mockFuncs<Z2, Y>::template stdcallexpectation7<X, A, B, C, D, E, F, G>;
		BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
			funcIndex.first,
			funcIndex.second,
			reinterpret_cast<void (base_mock::*)()>(mfp), X);
		TCall<Y, A, B, C, D, E, F, G> *call = new TCall<Y, A, B, C, D, E, F, G>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
		return *call;
}
template <int X, typename Z2, typename Y, typename Z,
	typename A, typename B, typename C, typename D,
	typename E, typename F, typename G, typename H>
TCall<Y,A,B,C,D,E,F,G,H> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
		std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A, B, C, D, E, F, G, H))func);
		Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A, B, C, D, E, F, G, H);
		mfp = &mockFuncs<Z2, Y>::template stdcallexpectation8<X, A, B, C, D, E, F, G, H>;
		BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
			funcIndex.first,
			funcIndex.second,
			reinterpret_cast<void (base_mock::*)()>(mfp), X);
		TCall<Y, A, B, C, D, E, F, G, H> *call = new TCall<Y, A, B, C, D, E, F, G, H>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
		return *call;
}
template <int X, typename Z2, typename Y, typename Z,
	typename A, typename B, typename C, typename D,
	typename E, typename F, typename G, typename H,
	typename I>
TCall<Y,A,B,C,D,E,F,G,H,I> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
		std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A, B, C, D, E, F, G, H, I))func);
		Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A, B, C, D, E, F, G, H, I);
		mfp = &mockFuncs<Z2, Y>::template stdcallexpectation9<X, A, B, C, D, E, F, G, H, I>;
		BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
			funcIndex.first,
			funcIndex.second,
			reinterpret_cast<void (base_mock::*)()>(mfp), X);
		TCall<Y, A, B, C, D, E, F, G, H, I> *call = new TCall<Y, A, B, C, D, E, F, G, H, I>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
		return *call;
}
template <int X, typename Z2, typename Y, typename Z,
	typename A, typename B, typename C, typename D,
	typename E, typename F, typename G, typename H,
	typename I, typename J>
TCall<Y,A,B,C,D,E,F,G,H,I,J> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
		std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A, B, C, D, E, F, G, H, I, J))func);
		Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A, B, C, D, E, F, G, H, I, J);
		mfp = &mockFuncs<Z2, Y>::template stdcallexpectation10<X, A, B, C, D, E, F, G, H, I, J>;
		BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
			funcIndex.first,
			funcIndex.second,
			reinterpret_cast<void (base_mock::*)()>(mfp), X);
		TCall<Y, A, B, C, D, E, F, G, H, I, J> *call = new TCall<Y, A, B, C, D, E, F, G, H, I, J>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
		return *call;
}
template <int X, typename Z2, typename Y, typename Z,
	typename A, typename B, typename C, typename D,
	typename E, typename F, typename G, typename H,
	typename I, typename J, typename K>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
		std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A, B, C, D, E, F, G, H, I, J, K))func);
		Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A, B, C, D, E, F, G, H, I, J, K);
		mfp = &mockFuncs<Z2, Y>::template stdcallexpectation11<X, A, B, C, D, E, F, G, H, I, J, K>;
		BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
			funcIndex.first,
			funcIndex.second,
			reinterpret_cast<void (base_mock::*)()>(mfp), X);
		TCall<Y, A, B, C, D, E, F, G, H, I, J, K> *call = new TCall<Y, A, B, C, D, E, F, G, H, I, J, K>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
		return *call;
}
template <int X, typename Z2, typename Y, typename Z,
	typename A, typename B, typename C, typename D,
	typename E, typename F, typename G, typename H,
	typename I, typename J, typename K, typename L>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
		std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A, B, C, D, E, F, G, H, I, J, K, L))func);
		Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A, B, C, D, E, F, G, H, I, J, K, L);
		mfp = &mockFuncs<Z2, Y>::template stdcallexpectation12<X, A, B, C, D, E, F, G, H, I, J, K, L>;
		BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
			funcIndex.first,
			funcIndex.second,
			reinterpret_cast<void (base_mock::*)()>(mfp), X);
		TCall<Y, A, B, C, D, E, F, G, H, I, J, K, L> *call = new TCall<Y, A, B, C, D, E, F, G, H, I, J, K, L>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
		return *call;
}
template <int X, typename Z2, typename Y, typename Z,
	typename A, typename B, typename C, typename D,
	typename E, typename F, typename G, typename H,
	typename I, typename J, typename K, typename L,
	typename M>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
		std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A, B, C, D, E, F, G, H, I, J, K, L, M))func);
		Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A, B, C, D, E, F, G, H, I, J, K, L, M);
		mfp = &mockFuncs<Z2, Y>::template stdcallexpectation13<X, A, B, C, D, E, F, G, H, I, J, K, L, M>;
		BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
			funcIndex.first,
			funcIndex.second,
			reinterpret_cast<void (base_mock::*)()>(mfp), X);
		TCall<Y, A, B, C, D, E, F, G, H, I, J, K, L, M> *call = new TCall<Y, A, B, C, D, E, F, G, H, I, J, K, L, M>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
		return *call;
}
template <int X, typename Z2, typename Y, typename Z,
	typename A, typename B, typename C, typename D,
	typename E, typename F, typename G, typename H,
	typename I, typename J, typename K, typename L,
	typename M, typename N>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
		std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A, B, C, D, E, F, G, H, I, J, K, L, M, N))func);
		Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A, B, C, D, E, F, G, H, I, J, K, L, M, N);
		mfp = &mockFuncs<Z2, Y>::template stdcallexpectation14<X, A, B, C, D, E, F, G, H, I, J, K, L, M, N>;
		BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
			funcIndex.first,
			funcIndex.second,
			reinterpret_cast<void (base_mock::*)()>(mfp), X);
		TCall<Y, A, B, C, D, E, F, G, H, I, J, K, L, M, N> *call = new TCall<Y, A, B, C, D, E, F, G, H, I, J, K, L, M, N>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
		return *call;
}
template <int X, typename Z2, typename Y, typename Z,
	typename A, typename B, typename C, typename D,
	typename E, typename F, typename G, typename H,
	typename I, typename J, typename K, typename L,
	typename M, typename N, typename O>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
		std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O))func);
		Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O);
		mfp = &mockFuncs<Z2, Y>::template stdcallexpectation15<X, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O>;
		BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
			funcIndex.first,
			funcIndex.second,
			reinterpret_cast<void (base_mock::*)()>(mfp), X);
		TCall<Y, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O> *call = new TCall<Y, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
		return *call;
}

template <int X, typename Z2, typename Y, typename Z,
	typename A, typename B, typename C, typename D,
	typename E, typename F, typename G, typename H,
	typename I, typename J, typename K, typename L,
	typename M, typename N, typename O, typename P>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
		std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P))func);
		Y(__stdcall mockFuncs<Z2, Y>::*mfp)(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P);
		mfp = &mockFuncs<Z2, Y>::template stdcallexpectation16<X, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>;
		BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
			funcIndex.first,
			funcIndex.second,
			reinterpret_cast<void (base_mock::*)()>(mfp), X);
		TCall<Y, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> *call = new TCall<Y, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	addCall( call, expect );
		return *call;
}
#endif

#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
template <int X, typename Y>
TCall<Y> &MockRepository::RegisterExpect_(Y (*func)(), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)();
  fp = &mockFuncs<char, Y>::template static_expectation0<X>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y> *call = new TCall<Y>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y, typename A>
TCall<Y,A> &MockRepository::RegisterExpect_(Y (*func)(A), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A);
  fp = &mockFuncs<char, Y>::template static_expectation1<X,A>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A> *call = new TCall<Y,A>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B>
TCall<Y,A,B> &MockRepository::RegisterExpect_(Y (*func)(A,B), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A,B);
  fp = &mockFuncs<char, Y>::template static_expectation2<X,A,B>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B> *call = new TCall<Y,A,B>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C>
TCall<Y,A,B,C> &MockRepository::RegisterExpect_(Y (*func)(A,B,C), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A,B,C);
  fp = &mockFuncs<char, Y>::template static_expectation3<X,A,B,C>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C> *call = new TCall<Y,A,B,C>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D>
TCall<Y,A,B,C,D> &MockRepository::RegisterExpect_(Y (*func)(A,B,C,D), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A,B,C,D);
  fp = &mockFuncs<char, Y>::template static_expectation4<X,A,B,C,D>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D> *call = new TCall<Y,A,B,C,D>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E>
TCall<Y,A,B,C,D,E> &MockRepository::RegisterExpect_(Y (*func)(A,B,C,D,E), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A,B,C,D,E);
  fp = &mockFuncs<char, Y>::template static_expectation5<X,A,B,C,D,E>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E> *call = new TCall<Y,A,B,C,D,E>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F>
TCall<Y,A,B,C,D,E,F> &MockRepository::RegisterExpect_(Y (*func)(A,B,C,D,E,F), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A,B,C,D,E,F);
  fp = &mockFuncs<char, Y>::template static_expectation6<X,A,B,C,D,E,F>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F> *call = new TCall<Y,A,B,C,D,E,F>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G>
TCall<Y,A,B,C,D,E,F,G> &MockRepository::RegisterExpect_(Y (*func)(A,B,C,D,E,F,G), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A,B,C,D,E,F,G);
  fp = &mockFuncs<char, Y>::template static_expectation7<X,A,B,C,D,E,F,G>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G> *call = new TCall<Y,A,B,C,D,E,F,G>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
   addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H>
TCall<Y,A,B,C,D,E,F,G,H> &MockRepository::RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A,B,C,D,E,F,G,H);
  fp = &mockFuncs<char, Y>::template static_expectation8<X,A,B,C,D,E,F,G,H>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H> *call = new TCall<Y,A,B,C,D,E,F,G,H>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I>
TCall<Y,A,B,C,D,E,F,G,H,I> &MockRepository::RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A,B,C,D,E,F,G,H,I);
  fp = &mockFuncs<char, Y>::template static_expectation9<X,A,B,C,D,E,F,G,H,I>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I> *call = new TCall<Y,A,B,C,D,E,F,G,H,I>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J>
TCall<Y,A,B,C,D,E,F,G,H,I,J> &MockRepository::RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I,J), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A,B,C,D,E,F,G,H,I,J);
  fp = &mockFuncs<char, Y>::template static_expectation10<X,A,B,C,D,E,F,G,H,I,J>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I,J> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &MockRepository::RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I,J,K), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A,B,C,D,E,F,G,H,I,J,K);
  fp = &mockFuncs<char, Y>::template static_expectation11<X,A,B,C,D,E,F,G,H,I,J,K>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I,J,K> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &MockRepository::RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I,J,K,L), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A,B,C,D,E,F,G,H,I,J,K,L);
  fp = &mockFuncs<char, Y>::template static_expectation12<X,A,B,C,D,E,F,G,H,I,J,K,L>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &MockRepository::RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I,J,K,L,M), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A,B,C,D,E,F,G,H,I,J,K,L,M);
  fp = &mockFuncs<char, Y>::template static_expectation13<X,A,B,C,D,E,F,G,H,I,J,K,L,M>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &MockRepository::RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A,B,C,D,E,F,G,H,I,J,K,L,M,N);
  fp = &mockFuncs<char, Y>::template static_expectation14<X,A,B,C,D,E,F,G,H,I,J,K,L,M,N>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N, typename O>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &MockRepository::RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O);
  fp = &mockFuncs<char, Y>::template static_expectation15<X,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}

template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N, typename O, typename P>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &MockRepository::RegisterExpect_(Y (*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P);
  fp = &mockFuncs<char, Y>::template static_expectation16<X,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}

#if defined(_MSC_VER) && !defined(_WIN64)
template <int X, typename Y>
TCall<Y> &MockRepository::RegisterExpect_(Y (__stdcall *func)(), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)();
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation0<X>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y> *call = new TCall<Y>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y, typename A>
TCall<Y,A> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation1<X,A>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A> *call = new TCall<Y,A>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B>
TCall<Y,A,B> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A,B), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A,B);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation2<X,A,B>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B> *call = new TCall<Y,A,B>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C>
TCall<Y,A,B,C> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A,B,C), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A,B,C);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation3<X,A,B,C>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C> *call = new TCall<Y,A,B,C>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D>
TCall<Y,A,B,C,D> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A,B,C,D), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A,B,C,D);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation4<X,A,B,C,D>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D> *call = new TCall<Y,A,B,C,D>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E>
TCall<Y,A,B,C,D,E> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A,B,C,D,E);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation5<X,A,B,C,D,E>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E> *call = new TCall<Y,A,B,C,D,E>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F>
TCall<Y,A,B,C,D,E,F> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A,B,C,D,E,F);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation6<X,A,B,C,D,E,F>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F> *call = new TCall<Y,A,B,C,D,E,F>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G>
TCall<Y,A,B,C,D,E,F,G> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A,B,C,D,E,F,G);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation7<X,A,B,C,D,E,F,G>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G> *call = new TCall<Y,A,B,C,D,E,F,G>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
   addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H>
TCall<Y,A,B,C,D,E,F,G,H> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A,B,C,D,E,F,G,H);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation8<X,A,B,C,D,E,F,G,H>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H> *call = new TCall<Y,A,B,C,D,E,F,G,H>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I>
TCall<Y,A,B,C,D,E,F,G,H,I> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A,B,C,D,E,F,G,H,I);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation9<X,A,B,C,D,E,F,G,H,I>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I> *call = new TCall<Y,A,B,C,D,E,F,G,H,I>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J>
TCall<Y,A,B,C,D,E,F,G,H,I,J> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I,J), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A,B,C,D,E,F,G,H,I,J);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation10<X,A,B,C,D,E,F,G,H,I,J>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I,J> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I,J,K), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A,B,C,D,E,F,G,H,I,J,K);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation11<X,A,B,C,D,E,F,G,H,I,J,K>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I,J,K> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I,J,K,L), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A,B,C,D,E,F,G,H,I,J,K,L);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation12<X,A,B,C,D,E,F,G,H,I,J,K,L>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I,J,K,L,M), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A,B,C,D,E,F,G,H,I,J,K,L,M);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation13<X,A,B,C,D,E,F,G,H,I,J,K,L,M>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A,B,C,D,E,F,G,H,I,J,K,L,M,N);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation14<X,A,B,C,D,E,F,G,H,I,J,K,L,M,N>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N, typename O>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation15<X,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}

template <int X, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N, typename O, typename P>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &MockRepository::RegisterExpect_(Y (__stdcall *func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation16<X,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
#endif
#endif

template <int X, typename Z2, typename Y, typename Z>
TCall<Y> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y (Z2::*)())func);
	Y (mockFuncs<Z2, Y>::*mfp)();
	mfp = &mockFuncs<Z2, Y>::template expectation0<X>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y> *call = new TCall<Y>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);

	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z, typename A>
TCall<Y,A> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A);
	mfp = &mockFuncs<Z2, Y>::template expectation1<X,A>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A> *call = new TCall<Y,A>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z,
		  typename A, typename B>
TCall<Y,A,B> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A,B))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B);
	mfp = &mockFuncs<Z2, Y>::template expectation2<X,A,B>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B> *call = new TCall<Y,A,B>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z,
		  typename A, typename B, typename C>
TCall<Y,A,B,C> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A,B,C))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C);
	mfp = &mockFuncs<Z2, Y>::template expectation3<X,A,B,C>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C> *call = new TCall<Y,A,B,C>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z,
		  typename A, typename B, typename C, typename D>
TCall<Y,A,B,C,D> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A,B,C,D))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D);
	mfp = &mockFuncs<Z2, Y>::template expectation4<X,A,B,C,D>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D> *call = new TCall<Y,A,B,C,D>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z,
		  typename A, typename B, typename C, typename D,
		  typename E>
TCall<Y,A,B,C,D,E> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A,B,C,D,E))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E);
	mfp = &mockFuncs<Z2, Y>::template expectation5<X,A,B,C,D,E>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E> *call = new TCall<Y,A,B,C,D,E>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F>
TCall<Y,A,B,C,D,E,F> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A,B,C,D,E,F))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F);
	mfp = &mockFuncs<Z2, Y>::template expectation6<X,A,B,C,D,E,F>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F> *call = new TCall<Y,A,B,C,D,E,F>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G>
TCall<Y,A,B,C,D,E,F,G> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A,B,C,D,E,F,G))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G);
	mfp = &mockFuncs<Z2, Y>::template expectation7<X,A,B,C,D,E,F,G>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G> *call = new TCall<Y,A,B,C,D,E,F,G>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H>
TCall<Y,A,B,C,D,E,F,G,H> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A,B,C,D,E,F,G,H))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H);
	mfp = &mockFuncs<Z2, Y>::template expectation8<X,A,B,C,D,E,F,G,H>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H> *call = new TCall<Y,A,B,C,D,E,F,G,H>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I>
TCall<Y,A,B,C,D,E,F,G,H,I> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A,B,C,D,E,F,G,H,I))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I);
	mfp = &mockFuncs<Z2, Y>::template expectation9<X,A,B,C,D,E,F,G,H,I>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I> *call = new TCall<Y,A,B,C,D,E,F,G,H,I>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J>
TCall<Y,A,B,C,D,E,F,G,H,I,J> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A,B,C,D,E,F,G,H,I,J))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I,J);
	mfp = &mockFuncs<Z2, Y>::template expectation10<X,A,B,C,D,E,F,G,H,I,J>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I,J> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A,B,C,D,E,F,G,H,I,J,K))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I,J,K);
	mfp = &mockFuncs<Z2, Y>::template expectation11<X,A,B,C,D,E,F,G,H,I,J,K>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A,B,C,D,E,F,G,H,I,J,K,L))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I,J,K,L);
	mfp = &mockFuncs<Z2, Y>::template expectation12<X,A,B,C,D,E,F,G,H,I,J,K,L>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A,B,C,D,E,F,G,H,I,J,K,L,M))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I,J,K,L,M);
	mfp = &mockFuncs<Z2, Y>::template expectation13<X,A,B,C,D,E,F,G,H,I,J,K,L,M>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I,J,K,L,M,N);
	mfp = &mockFuncs<Z2, Y>::template expectation14<X,A,B,C,D,E,F,G,H,I,J,K,L,M,N>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}
template <int X, typename Z2, typename Y, typename Z,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N, typename O>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O);
	mfp = &mockFuncs<Z2, Y>::template expectation15<X,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}

template <int X, typename Z2, typename Y, typename Z,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N, typename O, typename P>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
	std::pair<int, int> funcIndex = virtual_index((Y(Z2::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P))func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P);
	mfp = &mockFuncs<Z2, Y>::template expectation16<X,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
						funcIndex.first, funcIndex.second,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);
	addCall( call, expect );
	return *call;
}

template <typename Z>
Z MockRepository::DoExpectation(base_mock *mock, std::pair<int, int> funcno, const base_tuple &tuple)
{
	for (std::list<Call *>::reverse_iterator i = neverCalls.rbegin(); i != neverCalls.rend(); ++i)
	{
		Call *call = *i;
		if ( matches( call, mock, funcno, tuple ) )
		{
			call->satisfied = true;
			RAISEEXCEPTION(ExpectationException(this, call->getArgs(), call->funcName));
		}
	}
   for (std::list<Call *>::reverse_iterator i = expectations.rbegin(); i != expectations.rend(); ++i)
	{
		Call *call = *i;
		if( isUnsatisfied( call, mock, funcno, tuple ) )
		{
			return doReturnCall<Z>( call, tuple );
		}
	}
	for (std::list<Call *>::reverse_iterator i = optionals.rbegin(); i != optionals.rend(); ++i)
	{
		Call *call = *i;
		if ( matches( call, mock, funcno, tuple ) )
		{
		   return doReturnCall<Z>( call, tuple );
		}
	}
	RAISEEXCEPTION(ExpectationException(this, &tuple, funcName(mock,funcno)));
}
template <typename base>
base *MockRepository::Mock() {
	mock<base> *m = new mock<base>(this);
		mocks.push_back(m);
	return reinterpret_cast<base *>(m);
}
inline std::ostream &operator<<(std::ostream &os, const Call &call)
{
	os << call.fileName << "(" << call.lineno << ") ";
	if (call.expectation == Once)
		os << "Expectation for ";
	else
		os << "Result set for ";

	os << call.funcName;

		if (call.getArgs())
				call.getArgs()->printTo(os);
		else
				os << "(...)";

		os << " on the mock at 0x" << call.mock << " was ";

	if (!call.satisfied)
		os << "not ";

	if (call.expectation == Once)
		os << "satisfied." << std::endl;
	else
		os << "used." << std::endl;

	return os;
}

inline std::ostream &operator<<(std::ostream &os, const MockRepository &repo)
{
   if (repo.expectations.size())
	{
		os << "Expectations set:" << std::endl;
		for (std::list<Call *>::const_iterator exp = repo.expectations.begin(); exp != repo.expectations.end(); ++exp)
			os << **exp;
		os << std::endl;
	}

	if (repo.neverCalls.size())
	{
		os << "Functions explicitly expected to not be called:" << std::endl;
		for (std::list<Call *>::const_iterator exp = repo.neverCalls.begin(); exp != repo.neverCalls.end(); ++exp)
			os << **exp;
		os << std::endl;
	}

	if (repo.optionals.size())
	{
		os << "Optional results set up:" << std::endl;
		for (std::list<Call *>::const_iterator exp = repo.optionals.begin(); exp != repo.optionals.end(); ++exp)
			os << **exp;
		os << std::endl;
	}
	return os;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifndef NO_HIPPOMOCKS_NAMESPACE
}

using HippoMocks::MockRepository;
using HippoMocks::DONTCARE_NAME;
using HippoMocks::Call;
using HippoMocks::Out;
using HippoMocks::In;
#endif

#undef DEBUGBREAK
#undef BASE_EXCEPTION
#undef RAISEEXCEPTION
#undef RAISELATENTEXCEPTION
#undef DONTCARE_NAME
#undef VIRT_FUNC_LIMIT
#undef EXTRA_DESTRUCTOR
#undef FUNCTION_BASE
#undef FUNCTION_STRIDE
#undef CFUNC_MOCK_PLATFORMIS64BIT

#endif

