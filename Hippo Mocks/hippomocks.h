#ifndef HIPPOMOCKS_H
#define HIPPOMOCKS_H

#include <list>
#include <map>

class MockRepository;

class base_mock {
public:
	void *vft;
	void (base_mock::*next_func)();
	void (base_mock::*funcs[10])();
	int curFuncNo;
	MockRepository *repo;
	void f0();
	void f1();
	void f2();
	void f3();
	void f4();
	void f5();
	void f6();
	void f7();
	void f8();
	void f9();
	base_mock(MockRepository *repo) : repo(repo), vft((void *)&funcs) {
		funcs[0] = &base_mock::f0;
		funcs[1] = &base_mock::f1;
		funcs[2] = &base_mock::f2;
		funcs[3] = &base_mock::f3;
		funcs[4] = &base_mock::f4;
		funcs[5] = &base_mock::f5;
		funcs[6] = &base_mock::f6;
		funcs[7] = &base_mock::f7;
		funcs[8] = &base_mock::f8;
		funcs[9] = &base_mock::f9;
	}
	void (base_mock::*base_func(int index))();
};

class ExpectationException : public std::exception {
public:
	const char *what() const { return "Expectation was violated!"; }
};

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
		const tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &o = (const tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &)bo;
		return (a == o.a &&
				b == o.b &&
				c == o.c &&
				d == o.d &&
				e == o.e &&
				f == o.f &&
				g == o.g &&
				h == o.h &&
				i == o.i &&
				j == o.j &&
				k == o.k &&
				l == o.l &&
				m == o.m &&
				n == o.n &&
				this->o == o.o &&
				p == o.p);
	}
};

template <class T>
class mock : public base_mock 
{
	//TODO: fill remaining with some kind of instance of T
	char remaining[sizeof(T)];
public:
	mock(MockRepository *repo) 
		: base_mock(repo)
	{
		memset(remaining, 0, sizeof(remaining));
	}
	// for next year
#ifdef CPP0X
	template <int X, typename A...>
	void expectation(A... a)
	{
		repo->DoExpectation(this, X);
	}
#else
	template <int X>
	void expectation0()
	{
		repo->DoExpectation(this, X, new tuple<>());
	}
	template <int X, typename A>
	void expectation1(A a)
	{
		repo->DoExpectation(this, X, new tuple<A>(a));
	}
	template <int X, typename A, typename B>
	void expectation2(A a, B b)
	{
		repo->DoExpectation(this, X, new tuple<A>(b));
	}
#endif
};

class MockRepository {
private:
	std::list<base_mock *> mocks;
	std::list<std::pair<base_mock *, std::pair<int, base_tuple *> > > expectations;
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
#define RegisterExpectation RegisterExpect_<__COUNTER__>
	template <int X, typename Y, typename Z>
	void RegisterExpect_(Z *mck, Y (Z::*func)()) {
		mock<Z> *zMock = reinterpret_cast<mock<Z> *>(mck);
		zMock->next_func = (void (base_mock::*)())&mock<Z>::expectation0<X>;
		(mck->*func)();
	}
	template <int X, typename Y, typename Z, typename A>
	void RegisterExpect_(Z *mck, Y (Z::*func)(A), A a) {
		mock<Z> *zMock = reinterpret_cast<mock<Z> *>(mck);
		zMock->next_func = (void (base_mock::*)())&mock<Z>::expectation1<X,A>;
		(mck->*func)(a);
	}
	template <int X, typename Y, typename Z, typename A, typename B>
	void RegisterExpect_(Z *mck, Y (Z::*func)(A, B), A a, B b) {
		mock<Z> *zMock = reinterpret_cast<mock<Z> *>(mck);
		zMock->next_func = (void (base_mock::*)())&mock<Z>::expectation2<X,A,B>;
		(mck->*func)(a, b);
	}
#endif
	void DoExpectation(base_mock *mock, int funcno, base_tuple *tuple) 
	{
		switch(state)
		{
		case Record: 
			expectations.push_back(std::make_pair(mock, std::make_pair(funcno, tuple))); 
			break;
		case Playback: 
			{
				if (expectations.size() == 0)
					throw ExpectationException();

				std::pair<base_mock *, std::pair<int, base_tuple *> > call = expectations.front(); 
				expectations.pop_front(); 
				if (mock != call.first || 
					funcno != call.second.first ||
					!((*tuple) == (*call.second.second))) throw ExpectationException();
				delete call.second.second;
			}
			break;
		case Verified:
			throw ExpectationException();
		}
	}
	MockRepository() 
		: state(Record)
	{
		
	}
	~MockRepository() 
	{
	}
	template <typename base>
	base *newMock() {
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

class Expect_ 
{
public:
	template <typename T>
	Expect_ &Call(T) { 
		return *this;
	}
};

extern Expect_ Expect;

#endif

