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

#include <cstdio>
#include <vector>
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

#include "detail/replace.h"

#ifndef NO_HIPPOMOCKS_NAMESPACE
namespace HippoMocks {
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

const RegistrationType Any = RegistrationType( 1, (std::numeric_limits<unsigned>::max)());
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

class DontCare { static DontCare& instance(); };
static DontCare DONTCARE_NAME;
// This silences the not-used warning.
inline DontCare& DontCare::instance() { return DONTCARE_NAME; }

template <typename T>
struct OutParam: public DontCare
{
  explicit OutParam(T val): value(val) {}
  T value;
};

template <typename T>
OutParam<T> Out(T t) { return OutParam<T>(t); }

template <typename T>
struct InParam : public DontCare
{
  explicit InParam(T* val) : value(val)
  {
  }
  T* value;
};

template <typename T>
InParam<T> In(T& t) { return InParam<T>(&t); }

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
static inline bool operator==(const DontCare&, const T&)
{
  return true;
}

template <typename T, typename U>
static inline bool operator==(const std::reference_wrapper<U> &a, typename with_const<T>::type b)
{
  return &a.get() == &b;
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

template <int index, int limit, typename Tuple>
struct argumentPrinter {
  static void Print(std::ostream& os, const Tuple& t) {
    if (index != 0) os << ",";
    os << std::get<index>(t);
    argumentPrinter<index+1, limit, Tuple>::Print(os, t);
  }
};
template <int limit, typename Tuple>
struct argumentPrinter<limit, limit, Tuple> {
  static void Print(std::ostream&, const Tuple&) {}
};

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
  std::vector<TypeDestructable *> members;
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
    for (std::vector<TypeDestructable *>::iterator i = members.begin(); i != members.end(); ++i)
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

template <class T>
class ReturnValueWrapper {
public:
  virtual ~ReturnValueWrapper() {}
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

template <typename T>
class ReturnValueHandle {
public:
  ReturnValueHandle() : wrapper(nullptr) {}
  ~ReturnValueHandle() {}
  ReturnValueWrapper<T>* wrapper;
  void operator=(ReturnValueWrapper<T>* newValue) {
    delete wrapper;
    wrapper = newValue;
  }
  T value() {
    return wrapper->value();
  }
  bool set() {
    return wrapper != nullptr;
  }
};

template <>
class ReturnValueHandle<void> {
public:
  void value() {}
  bool set() { return true; }
};

//Call wrapping
class Call {
public:
  base_mock *mock;
  std::pair<int, int> funcIndex;
  std::vector<Call *> previousCalls;
  unsigned int called;
  RegistrationType expectation;
  int lineno;
  const char *funcName;
  const char *fileName;
  virtual ~Call() {}
  // This function checks if the call we've now received applies to this mock and function. If so we can use the type info.
  bool applies(base_mock* rhsMock, std::pair<int, int> rhsIndex) {
    return mock == rhsMock &&
           funcIndex == rhsIndex &&
           previousCallsSatisfied();
  }
  bool isSatisfied() const {
    return called >= expectation.minimum && previousCallsSatisfied();
  }
  virtual void printArgs(std::ostream& os) const = 0;
private:
  inline bool previousCallsSatisfied() const
  {
    for (auto& c : previousCalls) {
      if (!c->isSatisfied()) return false;
    }
    return true;
  }
protected:
  Call(RegistrationType expect, base_mock *baseMock, const std::pair<int, int> &index, int X, const char *func, const char *file)
    : mock(baseMock)
    , funcIndex(index)
    , called(0)
    , expectation(expect)
    , lineno(X)
    , funcName(func)
    , fileName(file)
  {
  }
};

std::ostream &operator<<(std::ostream &os, const Call &call);

template <int...> struct seq {};
template <int N, int... Ns> struct gen : gen<N-1, N-1, Ns...> {};
template <int... Ns> struct gen<0, Ns...> {
  typedef seq<Ns...> type;
};

template <typename Y, typename... Args, int... Nums>
Y _invoke(const std::function<Y(Args...)> &func, seq<Nums...>, const std::tuple<Args...>& args) {
  return func(std::get<Nums>(args)...);
}

template <typename Y, typename... Args>
Y invoke(const std::function<Y(Args...)> &func, const std::tuple<Args...>& args) {
  return _invoke<Y, Args...>(func, typename gen<sizeof...(Args)>::type(), args);
}

template <typename A, typename B>
struct assign_single {
  void operator()(A, B) {}
};
template <typename A, typename B>
struct assign_single<InParam<A>&, B&> {
  void operator()(InParam<A> a, B b) {
    *a.value = b;
  }
};
template <typename A, typename B>
struct assign_single<OutParam<A>&, B&> {
  void operator()(OutParam<A> a, B& b) {
    b = a.value;
  }
};
template <typename A, typename B>
struct assign_single<OutParam<A>&, B*&> {
  void operator()(OutParam<A> a, B*& b) {
    *b = a.value;
  }
};

template <typename A, typename B>
void assignTo(A& a, B& b) {
  assign_single<A&, B&>()(a, b);
}

template <int index, int limit, typename ArgTuple, typename Tuple>
struct assigner {
  static void Assign(ArgTuple& arg, Tuple& t) {
    assignTo(std::get<index>(arg), std::get<index>(t));
    assigner<index+1, limit, ArgTuple&, Tuple&>::Assign(arg, t);
  }
};
template <int limit, typename ArgTuple, typename Tuple>
struct assigner<limit, limit, ArgTuple, Tuple> {
  static void Assign(ArgTuple&, Tuple&) {}
};

template <typename... Args>
struct ComparableTupleBase {
public:
  virtual ~ComparableTupleBase() {}
  virtual void print(std::ostream& os) const = 0;
  virtual bool equals(const std::tuple<Args...>& rhs) = 0;
  virtual void assignInOut(std::tuple<Args...>& rhs) = 0;
};
template <typename tuple, typename... Args>
struct ComparableTuple;
template <typename... Args, typename... CArgs>
struct ComparableTuple<std::tuple<Args...>, CArgs...> : public std::tuple<CArgs...>, public ComparableTupleBase<Args...> {
public:
  ComparableTuple(CArgs... args) 
  : std::tuple<CArgs...>(args...)
  {
  }
  ~ComparableTuple() {
  }
  void print(std::ostream& os) const override {
    os << "(";
    argumentPrinter<0, sizeof...(CArgs),std::tuple<CArgs...>>::Print(os, *this);
    os << ")";
  }
  bool equals(const std::tuple<Args...>& rhs) override {
    // this effectively makes operator== virtual
    return (*this) == rhs;
  }
  void assignInOut(std::tuple<Args...>& rhs) override {
    assigner<0, sizeof...(Args), std::tuple<CArgs...>, std::tuple<Args...>>::Assign(*this, rhs);
  }
};

template <typename A, typename B>
struct is_not_equal { typedef int type; };
template <typename A>
struct is_not_equal<A, A> {};

template <typename Y, typename... Args>
class TCall : public Call {
protected:
  ReturnValueHandle<Y> retVal;
  ComparableTupleBase<Args...> *args;
  std::function<Y(Args...)> doFunctor;
  std::function<bool(Args...)> matchFunctor;
#ifndef HM_NO_EXCEPTIONS
  ExceptionHolder* eHolder;
#endif
public:
  void printArgs(std::ostream& os) const override {
    if (sizeof...(Args) == 0)
      os << "()";
    else if (args)
      args->print(os);
    else
      os << "(...)";
  }
  TCall(RegistrationType expect, base_mock *baseMock, std::pair<int, int> index, int X, const char *func, const char *file) : Call(expect, baseMock, index, X, func ,file), args(0) 
#ifndef HM_NO_EXCEPTIONS
  , eHolder(0)
#endif
  {}
  ~TCall() { delete args; }
  // This function checks that, given this is a call for this function, whether this call struct matches your call input.
  bool matches(const std::tuple<Args...> &tupl) {
    return (!args || args->equals(tupl)) &&
           (!matchFunctor || invoke(matchFunctor, tupl));
  }
  // This function handles your call. You have to check it at least applies before you call this.
  Y handle(std::tuple<Args...>& args) {
    // If we have too many calls, this is the first to handle.
    ++called;
    if (called > expectation.maximum) {
      RAISEEXCEPTION(ExpectationException(MockRepoInstanceHolder<0>::instance, args, funcName));
      std::abort(); // There's no way to return a Y from here without knowing how to make one. Only way out is an exception, so if you don't have those...
    }

    // Handle in/out arguments
    if(this->args) {
      this->args->assignInOut(args);
    }

    // If there's a doFunctor to invoke, invoke it now. A retVal overrides the functor return.
    if (doFunctor) {
      if (!retVal.set()) {
        return invoke(doFunctor, args);
      }
      invoke(doFunctor, args);
    }

    // If we have an exception to throw, let's throw it.
    #ifndef HM_NO_EXCEPTIONS
    if (eHolder)
      eHolder->rethrow();
    #endif

    // If not, we have to have a return value to give back. Void is folded into this as always being set.
    if (!retVal.set()) {
      RAISEEXCEPTION(NoResultSetUpException(MockRepoInstanceHolder<0>::instance, args, funcName));
    }
    return retVal.value();
  }
  template <typename... CArgs>
  TCall<Y,Args...> &With(CArgs... args) {
    this->args = new ComparableTuple<std::tuple<Args...>, CArgs...>(args...);
    return *this;
  }
  TCall<Y,Args...> &After(Call &call) {
    previousCalls.push_back(&call);
    return *this;
  }
  template <typename T>
  TCall<Y,Args...> &Do(T function) { doFunctor = function; return *this; }
  template <typename T>
  TCall<Y,Args...> &Match(T function) { matchFunctor = function; return *this; }
  template <typename RY, typename OY = Y, typename = typename std::enable_if<!std::is_same<OY, void>::value, bool>::type> Call &Return(RY obj) { retVal = new ReturnValueWrapperCopy<Y, RY>(obj); return *this; }
#ifndef HM_NO_EXCEPTIONS
  template <typename Ex>
  Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
#endif
};

class MockRepository {
private:
  friend inline std::ostream &operator<<(std::ostream &os, const MockRepository &repo);
  std::vector<base_mock *> mocks;
  std::map<void (*)(), int> staticFuncMap;
#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
  std::vector<Replace *> staticReplaces;
#endif

  std::vector<Call *> neverCalls;
  std::vector<Call *> expectations;
  std::vector<Call *> optionals;
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
  template <int X, typename Y, typename... Args>
  TCall<Y, Args...> &RegisterExpect_(Y (*func)(Args...), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);

#if defined(_MSC_VER) && !defined(_WIN64)
  template <int X, typename Y, typename... Args>
  TCall<Y, Args...> &RegisterExpect_(Y (__stdcall *func)(Args...), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
#endif
#endif

  template <int X, typename Z2, typename Y, typename Z, typename... Args>
  TCall<Y,Args...> &RegisterExpect_(Z2 *mck, Y (Z::*func)(Args...), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);

  //GCC 3.x doesn't seem to understand overloading on const or non-const member function.
#if !defined(__GNUC__) || __GNUC__ > 3
  template <int X, typename Z2, typename Y, typename Z, typename... Args>
  TCall<Y,Args...> &RegisterExpect_(Z2 *mck, Y (Z::*func)(Args...) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) 
  { return RegisterExpect_<X>(mck, (Y(Z::*)(Args...))(func), expect, functionName ,fileName, lineNo); }
  template <int X, typename Z2, typename Y, typename Z, typename... Args>
  TCall<Y,Args...> &RegisterExpect_(Z2 *mck, Y (Z::*func)(Args...) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) 
  { return RegisterExpect_<X>(mck, (Y(Z::*)(Args...))(func), expect, functionName ,fileName, lineNo); }
  template <int X, typename Z2, typename Y, typename Z, typename... Args>
  TCall<Y,Args...> &RegisterExpect_(Z2 *mck, Y (Z::*func)(Args...) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
  { return RegisterExpect_<X>(mck, (Y(Z::*)(Args...))(func), expect, functionName ,fileName, lineNo); }
#endif

#if defined(_MSC_VER) && !defined(_WIN64)
  // COM only support - you can duplicate this for cdecl and fastcall if you want to, but those are not as common as COM.
  template <int X, typename Z2, typename Y, typename Z, typename... Args>
  TCall<Y,Args...> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::* func)(Args...), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo);
  template <int X, typename Z2, typename Y, typename Z>
  TCall<Y,Args...> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(Args...) volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) 
  { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(Args....))(func), expect, functionName ,fileName, lineNo); }
  template <int X, typename Z2, typename Y, typename Z>
  TCall<Y,Args...> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(Args...) const volatile, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) 
  { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(Args...))(func), expect, functionName ,fileName, lineNo); }
  template <int X, typename Z2, typename Y, typename Z>
  TCall<Y,Args...> &RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(Args...) const, RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo) 
  { return RegisterExpect_<X>(mck, (Y(__stdcall Z::*)(Args...))(func), expect, functionName ,fileName, lineNo); }
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


  const char *funcName( base_mock *mock, std::pair<int, int> funcno )
  {
    const char *name = NULL;
    for (std::vector<Call *>::iterator i = expectations.begin(); i != expectations.end() && !name; ++i)
    {
     Call *call = *i;
     if (call->mock == mock &&
         call->funcIndex == funcno)
     name = call->funcName;
    }
    for (std::vector<Call *>::iterator i = optionals.begin(); i != optionals.end() && !name; ++i)
    {
     Call *call = *i;
     if (call->mock == mock &&
         call->funcIndex == funcno)
     name = call->funcName;
    }
    for (std::vector<Call *>::iterator i = neverCalls.begin(); i != neverCalls.end() && !name; ++i)
    {
     Call *call = *i;
     if (call->mock == mock &&
         call->funcIndex == funcno)
         name = call->funcName;
    }
    return name;
  }

  template <typename Y, typename... Args>
  Y DoExpectation(base_mock *mock, std::pair<int, int> funcno, std::tuple<Args...> &tuple);

  template <typename... Args>
  inline void DoVoidExpectation(base_mock *mock, std::pair<int, int> funcno, std::tuple<Args...> &tuple)
  {
    for (std::vector<Call *>::reverse_iterator i = neverCalls.rbegin(); i != neverCalls.rend(); ++i)
    {
      Call* c = *i;
      if (!c->applies(mock, funcno)) continue;

      TCall<void, Args...>* tc = static_cast<TCall<void, Args...>*>(c);
      if (tc->matches(tuple)) {
        tc->handle(tuple);
        return;
      }
    }
    for (std::vector<Call *>::reverse_iterator i = expectations.rbegin(); i != expectations.rend(); ++i)
    {
      Call* c = *i;
      if (!c->applies(mock, funcno)) continue;

      TCall<void, Args...>* tc = static_cast<TCall<void, Args...>*>(c);
      if (!tc->isSatisfied() && tc->matches(tuple)) {
        tc->handle(tuple);
        return;
      }
    }
    for (std::vector<Call *>::reverse_iterator i = optionals.rbegin(); i != optionals.rend(); ++i)
    {
      Call* c = *i;
      if (!c->applies(mock, funcno)) continue;

      TCall<void, Args...>* tc = static_cast<TCall<void, Args...>*>(c);
      if (tc->matches(tuple)) {
        tc->handle(tuple);
        return;
      }
    }

    RAISEEXCEPTION(ExpectationException(MockRepoInstanceHolder<0>::instance, tuple, funcName(mock, funcno)));
  }
  MockRepository()
    : autoExpect(DEFAULT_AUTOEXPECT)
#ifndef HM_NO_EXCEPTIONS
    , latentException(0)
#endif
  {
    MockRepoInstanceHolder<0>::instance = this;
  }
  ~MockRepository() noexcept(false)
  {
    MockRepoInstanceHolder<0>::instance = nullptr;
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
        for (std::vector<base_mock *>::iterator i = mocks.begin(); i != mocks.end(); i++)
        {
          (*i)->destroy();
        }
#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
        for (std::vector<Replace *>::iterator i = staticReplaces.begin(); i != staticReplaces.end(); i++)
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
    for (std::vector<base_mock *>::iterator i = mocks.begin(); i != mocks.end(); i++)
    {
      (*i)->destroy();
    }
#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
    for (std::vector<Replace *>::iterator i = staticReplaces.begin(); i != staticReplaces.end(); i++)
    {
      delete *i;
    }
#endif
  }
  void reset()
  {
    for (auto e : expectations) delete e;
    for (auto n : neverCalls) delete n;
    for (auto o : optionals) delete o;
    expectations.clear();
    neverCalls.clear();
    optionals.clear();
    for (std::vector<base_mock *>::iterator i = mocks.begin(); i != mocks.end(); i++)
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

    for (std::vector<Call *>::iterator i = expectations.begin(); i != expectations.end(); i++)
    {
      if (!(*i)->isSatisfied()) {
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
    for (std::vector<Call *>::iterator i = expectations.begin(); i != expectations.end(); i++)
    {
      if ((*i)->mock == (base_mock *)obj &&
        !(*i)->isSatisfied() )
      {
        RAISEEXCEPTION(CallMissingException(this));
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
  template <int X, typename... Args>
  Y expectation(Args... args)
  {
    std::tuple<Args...> argT(args...);
    mock<Z> *realMock = mock<Z>::getRealThis();
    if (realMock->isZombie)
      RAISEEXCEPTION(ZombieMockException(realMock->repo));
    MockRepository *repo = realMock->repo;
    return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), argT);
  }
  template <int X, typename... Args>
  static Y static_expectation(Args... args)
  {
    std::tuple<Args...> argT(args...);
    return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), argT);
  }
#ifdef _MSC_VER
  template <int X, typename... Args>
  Y __stdcall stdcallexpectation(Args... args)
  {
    std::tuple<Args...> argT(args...);
    mock<Z> *realMock = mock<Z>::getRealThis();
    if (realMock->isZombie)
      RAISEEXCEPTION(ZombieMockException(realMock->repo));
    MockRepository *repo = realMock->repo;
    return repo->template DoExpectation<Y>(realMock, realMock->translateX(X), argT);
  }
#if defined(_MSC_VER) && !defined(_WIN64)
  template <int X, typename... Args>
  static Y __stdcall static_stdcallexpectation(Args... args)
  {
    std::tuple<Args...> argT(args...);
    return MockRepoInstanceHolder<0>::instance->template DoExpectation<Y>(NULL, std::pair<int, int>(0, X), argT);
  }
#endif
#endif
};

template <typename Z>
class mockFuncs<Z, void> : public mock<Z> {
private:
  mockFuncs();
public:
  template <int X, typename... Args>
  void expectation(Args... args)
  {
    std::tuple<Args...> argT(args...);
    mock<Z> *realMock = mock<Z>::getRealThis();
    if (realMock->isZombie)
      RAISEEXCEPTION(ZombieMockException(realMock->repo));
    MockRepository *repo = realMock->repo;
    repo->DoVoidExpectation(realMock, realMock->translateX(X), argT);
  }
  template <int X, typename... Args>
  static void static_expectation(Args... args)
  {
    std::tuple<Args...> argT(args...);
    MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), argT);
  }

#ifdef _MSC_VER
  template <int X, typename... Args>
  void __stdcall stdcallexpectation(Args... args)
  {
    std::tuple<Args...> argT(args...);
    mock<Z> *realMock = mock<Z>::getRealThis();
    if (realMock->isZombie)
      RAISEEXCEPTION(ZombieMockException(realMock->repo));
    MockRepository *repo = realMock->repo;
    repo->DoVoidExpectation(this, mock<Z>::translateX(X), argT);
  }
#if defined(_MSC_VER) && !defined(_WIN64)
  template <int X, typename... Args>
  static void __stdcall static_stdcallexpectation(Args... args)
  {
    std::tuple<Args...> argT(args...);
    return MockRepoInstanceHolder<0>::instance->DoVoidExpectation(NULL, std::pair<int, int>(0, X), argT);
  }
#endif
#endif
};

template <typename T>
template <int X>
void mock<T>::mockedDestructor(int)
{
  std::tuple<> argT;
  repo->DoVoidExpectation(this, translateX(X), argT);
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
template <int X, typename Z2, typename Y, typename Z, typename... Args>
TCall<Y,Args...> &MockRepository::RegisterExpect_(Z2 *mck, Y (__stdcall Z::*func)(Args...), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  std::pair<int, int> funcIndex = virtual_index((Y(__stdcall Z2::*)(Args...))func);
  Y(__stdcall mockFuncs<Z2, Y>::*mfp)(Args...);
  mfp = &mockFuncs<Z2, Y>::template stdcallexpectation<X,Args...>;
  BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
    funcIndex.first,
    funcIndex.second,
    reinterpret_cast<void (base_mock::*)()>(mfp), X);
  TCall<Y,Args...> *call = new TCall<Y,Args...>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);

  addCall( call, expect );
  return *call;
}
#endif

#ifdef _HIPPOMOCKS__ENABLE_CFUNC_MOCKING_SUPPORT
template <int X, typename Y, typename... Args>
TCall<Y,Args...> &MockRepository::RegisterExpect_(Y (*func)(Args...), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (*fp)(Args...);
  fp = &mockFuncs<char, Y>::template static_expectation<X,Args...>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,Args...> *call = new TCall<Y,Args...>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}

#if defined(_MSC_VER) && !defined(_WIN64)
template <int X, typename Y, typename... Args>
TCall<Y,Args...> &MockRepository::RegisterExpect_(Y (__stdcall *func)(Args...), RegistrationType expect, const char *funcName, const char *fileName, unsigned long lineNo)
{
  Y (__stdcall *fp)(Args...);
  fp = &mockFuncs<char, Y>::template static_stdcallexpectation<X,Args...>;
  int index = BasicStaticRegisterExpect(reinterpret_cast<void (*)()>(func), reinterpret_cast<void (*)()>(fp),X);
  TCall<Y,Args...> *call = new TCall<Y,Args...>(expect, NULL, std::pair<int, int>(0, index), lineNo, funcName ,fileName);
  addCall( call, expect );
  return *call;
}
#endif
#endif

template <int X, typename Z2, typename Y, typename Z, typename... Args>
TCall<Y,Args...> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(Args...), RegistrationType expect, const char *functionName, const char *fileName, unsigned long lineNo)
{
  std::pair<int, int> funcIndex = virtual_index((Y (Z2::*)(Args...))func);
  Y (mockFuncs<Z2, Y>::*mfp)(Args...);
  mfp = &mockFuncs<Z2, Y>::template expectation<X,Args...>;
  BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck),
            funcIndex.first, funcIndex.second,
            reinterpret_cast<void (base_mock::*)()>(mfp),X);
  TCall<Y,Args...> *call = new TCall<Y,Args...>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, functionName ,fileName);

  addCall( call, expect );
  return *call;
}

template <typename Y, typename... Args>
Y MockRepository::DoExpectation(base_mock *mock, std::pair<int, int> funcno, std::tuple<Args...> &tuple)
{
  for (std::vector<Call *>::reverse_iterator i = neverCalls.rbegin(); i != neverCalls.rend(); ++i)
  {
    Call *c = *i;
    if (!c->applies(mock, funcno)) continue;

    TCall<Y, Args...>* tc = static_cast<TCall<Y, Args...>*>(c);
    if (tc->matches(tuple)) {
      return tc->handle(tuple);
    }
  }
  for (std::vector<Call *>::reverse_iterator i = expectations.rbegin(); i != expectations.rend(); ++i)
  {
    Call *c = *i;
    if (!c->applies(mock, funcno)) continue;

    TCall<Y, Args...>* tc = static_cast<TCall<Y, Args...>*>(c);
    if (!tc->isSatisfied() && tc->matches(tuple)) {
      return tc->handle(tuple);
    }
  }
  for (std::vector<Call *>::reverse_iterator i = optionals.rbegin(); i != optionals.rend(); ++i)
  {
    Call *c = *i;
    if (!c->applies(mock, funcno)) continue;

    TCall<Y, Args...>* tc = static_cast<TCall<Y, Args...>*>(c);
    if (tc->matches(tuple)) {
      return tc->handle(tuple);
    }
  }
  RAISEEXCEPTION(ExpectationException(MockRepoInstanceHolder<0>::instance, tuple, funcName(mock,funcno)));
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

  call.printArgs(os);

  os << " on the mock at 0x" << call.mock << " was ";

  if (!call.isSatisfied())
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
    for (std::vector<Call *>::const_iterator exp = repo.expectations.begin(); exp != repo.expectations.end(); ++exp)
      os << **exp;
    os << std::endl;
  }

  if (repo.neverCalls.size())
  {
    os << "Functions explicitly expected to not be called:" << std::endl;
    for (std::vector<Call *>::const_iterator exp = repo.neverCalls.begin(); exp != repo.neverCalls.end(); ++exp)
      os << **exp;
    os << std::endl;
  }

  if (repo.optionals.size())
  {
    os << "Optional results set up:" << std::endl;
    for (std::vector<Call *>::const_iterator exp = repo.optionals.begin(); exp != repo.optionals.end(); ++exp)
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

