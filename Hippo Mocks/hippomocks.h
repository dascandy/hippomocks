#ifndef HIPPOMOCKS_H
#define HIPPOMOCKS_H

#include <list>
#include <map>

class MockRepository;

class base_mock {};

class ExpectationException : public std::exception {
public:
	const char *what() const { return "Expectation was violated!"; }
};

class NotImplementedException : public std::exception {
public:
	const char *what() const { return "Function called with no expectations"; }
};

class NoResultSetUpException : public std::exception {};

class NullType 
{
public: 
	bool operator==(const NullType &) const 
	{
		return true; 
	}
};

class base_tuple 
{
protected:
	base_tuple() 
	{
	} 
public: 
	virtual ~base_tuple() 
	{
	}
	virtual bool operator==(const base_tuple &) const = 0;
};

template <typename A = NullType, typename B = NullType, typename C = NullType, typename D = NullType, 
		  typename E = NullType, typename F = NullType, typename G = NullType, typename H = NullType, 
		  typename I = NullType, typename J = NullType, typename K = NullType, typename L = NullType, 
		  typename M = NullType, typename N = NullType, typename O = NullType, typename P = NullType>
class tuple : public base_tuple
{
private:
	A a;
	B b;
	C c;
	D d;
	E e;
	F f;
	G g;
	H h;
	I i;
	J j;
	K k;
	L l;
	M m;
	N n;
	O o;
	P p;
public:
	tuple(A a = A(), B b = B(), C c = C(), D d = D(), E e = E(), F f = F(), G g = G(), H h = H(),
		  I i = I(), J j = J(), K k = K(), L l = L(), M m = M(), N n = N(), O o = O(), P p = P())
		  : a(a), b(b), c(c), d(d), e(e), f(f), g(g), h(h), i(i), j(j), k(k), l(l), m(m), n(n), o(o), p(p)
	{}
	bool operator==(const base_tuple &bo) const {
		const tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &to = (const tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &)bo;
		return (a == to.a &&
				b == to.b &&
				c == to.c &&
				d == to.d &&
				e == to.e &&
				f == to.f &&
				g == to.g &&
				h == to.h &&
				i == to.i &&
				j == to.j &&
				k == to.k &&
				l == to.l &&
				m == to.m &&
				n == to.n &&
				o == to.o &&
				p == to.p);
	}
};

template <class T>
class mock : public base_mock 
{
	friend class MockRepository;
	static const int VIRT_FUNC_LIMIT = 1024;
	void *vft;
	//TODO: fill remaining with some kind of instance of T
	void (mock<T>::*funcs[VIRT_FUNC_LIMIT])();
protected:
	MockRepository *repo;
private:
	char remaining[sizeof(T)];
	void NotImplemented() { throw NotImplementedException(); }
public:
	int funcMap[VIRT_FUNC_LIMIT];
	mock(MockRepository *repo) 
		: repo(repo), vft((void *)&funcs)
	{
		for (int i = 0; i < VIRT_FUNC_LIMIT; i++) 
		{
			funcs[i] = &mock<T>::NotImplemented;
			funcMap[i] = -1;
		}
		//TODO: replace remaining with instance of T (somehow)
		memset(remaining, 0, sizeof(remaining));
	}
	int translateX(int x) 
	{
		for (int i = 0; i < VIRT_FUNC_LIMIT; i++) 
		{
			if (funcMap[i] == x) return i;
		}
		return -1;
	}
};

template <typename Z, typename Y>
class mockFuncs : public mock<Z> 
{
public:
	template <int X>
	Y expectation0()
	{
		return repo->DoExpectation<Y>(this, translateX(X), new tuple<>());
	}
	template <int X, typename A>
	Y expectation1(A a)
	{
		return repo->DoExpectation<Y>(this, translateX(X), new tuple<A>(a));
	}
	template <int X, typename A, typename B>
	Y expectation2(A a, B b)
	{
		return repo->DoExpectation<Y>(this, translateX(X), new tuple<A>(b));
	}
};

template <typename Z>
class mockFuncs<Z, void> : public mock<Z> {
public:
	template <int X>
	void expectation0()
	{
		repo->DoExpectation<void>(this, translateX(X), new tuple<>());
	}
	template <int X, typename A>
	void expectation1(A a)
	{
		repo->DoExpectation<void>(this, translateX(X), new tuple<A>(a));
	}
	template <int X, typename A, typename B>
	void expectation2(A a, B b)
	{
		repo->DoExpectation<void>(this, translateX(X), new tuple<A>(b));
	}
};

class ExceptionHolder 
{ 
public:
	virtual void rethrow() = 0; 
};

template <class T>
class ExceptionWrapper : public ExceptionHolder {
	T exception;
public:
	ExceptionWrapper(T exception) : exception(exception) {}
	void rethrow() { throw exception; }
};

class Call {
public:
	virtual bool matchesArgs(base_tuple *tuple) = 0;
	bool isExpectation() { return expectation; }
	void *retVal;
	ExceptionHolder *eHolder;
	base_mock *mock;
	int funcIndex;
protected:
	bool expectation;
	Call(bool expectation, base_mock *mock, int funcIndex) 
		: retVal(0), 
		eHolder(0), 
		mock(mock), 
		funcIndex(funcIndex), 
		expectation(expectation) 
	{
	}
};

template <class Y>
class Call0 : public Call {
public:
	Call0(bool expectation, base_mock *mock, int funcIndex) : Call(expectation, mock, funcIndex) {}
	bool matchesArgs(base_tuple *tuple) { return true; }
	Call0<Y> &Return(Y obj) { retVal = new Y(obj); return *this; }
	template <typename E>
	Call0<Y> &Throw(E exception) { eHolder = new ExceptionWrapper<E>(exception); return *this; }
};

template <>
class Call0<void> : public Call {
public:
	Call0(bool expectation, base_mock *mock, int funcIndex) : Call(expectation, mock, funcIndex) {}
	bool matchesArgs(base_tuple *tuple) { return true; }
	template <typename E>
	Call0<void> &Throw(E exception) { eHolder = new ExceptionWrapper<E>(exception); return *this; }
};

template <class Y, class A>
class Call1 : public Call {
private:
	tuple<A> *args;
public:
	Call1(bool expectation, base_mock *mock, int funcIndex) : Call(expectation, mock, funcIndex), args(0) {}
	bool matchesArgs(base_tuple *tupl) { return !args || *args == *reinterpret_cast<tuple<A> *>(tupl); }
	Call1<Y,A> &With(A a) { args = new tuple<A>(a); return *this; }
	Call1<Y,A> &Return(Y obj) { retVal = new Y(obj); return *this; }
	template <typename E>
	Call1<Y,A> &Throw(E exception) { eHolder = new ExceptionWrapper<E>(exception); return *this; }
};

template <class A>
class Call1<void, A> : public Call {
private:
	tuple<A> *args;
public:
	Call1(bool expectation, base_mock *mock, int funcIndex) : Call(expectation, mock, funcIndex), args(0) {}
	bool matchesArgs(base_tuple *tupl) { return !args || *args == *reinterpret_cast<tuple<A> *>(tupl); }
	Call1<void,A> &With(A a) { args = new tuple<A>(a); return *this; }
	template <typename E>
	Call1<void,A> &Throw(E exception) { eHolder = new ExceptionWrapper<E>(exception); return *this; }
};

template <class Y, class A, class B>
class Call2 : public Call {
private:
	tuple<A,B> *args;
public:
	Call2(bool expectation, base_mock *mock, int funcIndex) : Call(expectation, mock, funcIndex), args(0) {}
	bool matchesArgs(base_tuple *tuple) { return !args || *args == *reinterpret_cast<tuple<A,B> *>(tuple); }
	Call2<Y,A,B> &With(A a, B b) { args = new tuple<A,B>(a,b); return *this; }
	Call2<Y,A,B> &Return(Y obj) { retVal = new Y(obj); return *this; }
	template <typename E>
	Call2<Y,A,B> &Throw(E exception) { eHolder = new ExceptionWrapper<E>(exception); return *this; }
};

template <class A, class B>
class Call2<void,A,B> : public Call {
private:
	tuple<A,B> *args;
public:
	Call2(bool expectation, base_mock *mock, int funcIndex) : Call(expectation, mock, funcIndex), args(0) {}
	bool matchesArgs(base_tuple *tuple) { return !args || *args == *reinterpret_cast<tuple<A,B> *>(tuple); }
	Call2<void,A,B> &With(A a, B b) { args = new tuple<A,B>(a,b); return *this; }
	template <typename E>
	Call2<void,A,B> &Throw(E exception) { eHolder = new ExceptionWrapper<E>(exception); return *this; }
};

class MockRepository {
private:
	std::list<base_mock *> mocks;
	std::list<Call *> expectations;
	enum { Record, Playback, Verified } state;
public:
	// for next year
#ifdef CPP0X
	template <typename Y, typename Z, typename A...>
	void RegisterExpectation(Z *mock, Y (Z::*func)(A...), A... a) {
		mock<Z> *zMock = reinterpret_cast<mock<Z> *>(mck);
		zMock->next_func = (void (base_mock::*)())&mock<Z>::expectation<X, A...>;
		(mock->*func)(a...);
	}
#else
	class func_index {
	public:
		virtual int f0() { return 0; }
		virtual int f1() { return 1; }
		virtual int f2() { return 2; }
		virtual int f3() { return 3; }
		virtual int f4() { return 4; }
		virtual int f5() { return 5; }
		virtual int f6() { return 6; }
		virtual int f7() { return 7; }
		virtual int f8() { return 8; }
		virtual int f9() { return 9; }
		int index(int (func_index::*func)()) { return (this->*func)(); }
	};
#define OnCall RegisterExpect_<__COUNTER__, false>
#define ExpectCall RegisterExpect_<__COUNTER__, true>
	template <int X, bool expect, typename Y, typename Z>
	Call0<Y> &RegisterExpect_(Z *mck, Y (Z::*func)()) {
		if (state != Record) throw ExpectationException();
		mock<Z> *zMock = reinterpret_cast<mock<Z> *>(mck);
		int funcIndex = ((&func_index())->*reinterpret_cast<int (func_index::*)()>(func))();
		if (zMock->funcMap[funcIndex] == -1)
		{
			zMock->funcs[funcIndex] = (void (base_mock::*)())&mockFuncs<Z, Y>::expectation0<X>;
			zMock->funcMap[funcIndex] = X;
		}
		Call0<Y> *call = new Call0<Y>(expect, zMock, funcIndex);
		expectations.push_back(call);
		return *call;
	}
	template <int X, bool expect, typename Y, typename Z, typename A>
	Call1<Y,A> &RegisterExpect_(Z *mck, Y (Z::*func)(A)) {
		if (state != Record) throw ExpectationException();
		mock<Z> *zMock = reinterpret_cast<mock<Z> *>(mck);
		int funcIndex = ((&func_index())->*reinterpret_cast<int (func_index::*)()>(func))();
		if (zMock->funcMap[funcIndex] == -1)
		{
			zMock->funcs[funcIndex] = (void (base_mock::*)())&mockFuncs<Z, Y>::expectation1<X,A>;
			zMock->funcMap[funcIndex] = X;
		}
		Call1<Y,A> *call = new Call1<Y,A>(expect, zMock, funcIndex);
		expectations.push_back(call);
		return *call;
	}
	template <int X, bool expect, typename Y, typename Z, typename A, typename B>
	Call2<Y,A,B> &RegisterExpect_(Z *mck, Y (Z::*func)(A, B)) {
		if (state != Record) throw ExpectationException();
		mock<Z> *zMock = reinterpret_cast<mock<Z> *>(mck);
		int funcIndex = ((&func_index())->*reinterpret_cast<int (func_index::*)()>(func))();
		if (zMock->funcMap[funcIndex] == -1)
		{
			zMock->funcs[funcIndex] = (void (base_mock::*)())&mockFuncs<Z, Y>::expectation2<X,A,B>;
			zMock->funcMap[funcIndex] = X;
		}
		Call2<Y,A,B> *call = new Call2<Y,A,B>(expect, zMock, funcIndex);
		expectations.push_back(call);
		return *call;
	}
#endif
	template <typename Z>
	Z DoExpectation(base_mock *mock, int funcno, base_tuple *tuple) 
	{
		Call *call = DoBasicExpectation(mock, funcno, tuple);

		if (call->retVal)
			return *((Z *)call->retVal);

		throw NoResultSetUpException();
	}
	template <>
	void DoExpectation(base_mock *mock, int funcno, base_tuple *tuple) 
	{
		DoBasicExpectation(mock, funcno, tuple);

		return;
	}

	Call *DoBasicExpectation(base_mock *mock, int funcno, base_tuple *tuple) 
	{
		if (state != Playback)
			throw ExpectationException();

		if (expectations.size() == 0)
			throw ExpectationException();

		Call *call = expectations.front(); 
		expectations.pop_front(); 

		if (mock != call->mock) 
			throw ExpectationException();

		if (funcno != call->funcIndex)
			throw ExpectationException();

		if (!call->matchesArgs(tuple))
			throw ExpectationException();

		if (call->eHolder)
			call->eHolder->rethrow();

		return call;
	}
	MockRepository() 
		: state(Record)
	{
	}
	~MockRepository() 
	{
	}
	template <typename base>
	base *InterfaceMock() {
		mock<base> *m = new mock<base>(this);
		mocks.push_back(m);
		return reinterpret_cast<base *>(m);
	}
	void ReplayAll() {
		state = Playback;
	}
	void VerifyAll() {
		if (expectations.size() > 0)
			throw ExpectationException();
		state = Verified;
	}
};

#endif

