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
	MockRepository *repo;
	char remaining[sizeof(T)];
	void NotImplemented() { throw NotImplementedException(); }
public:
	mock(MockRepository *repo) 
		: repo(repo), vft((void *)&funcs)
	{
		for (int i = 0; i < VIRT_FUNC_LIMIT; i++) {
			funcs[i] = &mock<T>::NotImplemented;
		}
		//TODO: replace remaining with instance of T (somehow)
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
#define RegisterExpectation RegisterExpect_<__COUNTER__>
	template <int X, typename Y, typename Z>
	void RegisterExpect_(Z *mck, Y (Z::*func)()) {
		mock<Z> *zMock = reinterpret_cast<mock<Z> *>(mck);
		int funcIndex = ((&func_index())->*reinterpret_cast<int (func_index::*)()>(func))();
		zMock->funcs[funcIndex] = (void (base_mock::*)())&mock<Z>::expectation0<X>;
		DoExpectation(zMock, X, NULL);
	}
	template <int X, typename Y, typename Z, typename A>
	void RegisterExpect_(Z *mck, Y (Z::*func)(A)) {
		mock<Z> *zMock = reinterpret_cast<mock<Z> *>(mck);
		int funcIndex = ((&func_index())->*reinterpret_cast<int (func_index::*)()>(func))();
		zMock->funcs[funcIndex] = (void (base_mock::*)())&mock<Z>::expectation1<X,A>;
		DoExpectation(zMock, X, NULL);
	}
	template <int X, typename Y, typename Z, typename A, typename B>
	void RegisterExpect_(Z *mck, Y (Z::*func)(A, B)) {
		mock<Z> *zMock = reinterpret_cast<mock<Z> *>(mck);
		int funcIndex = ((&func_index())->*reinterpret_cast<int (func_index::*)()>(func))();
		zMock->funcs[funcIndex] = (void (base_mock::*)())&mock<Z>::expectation2<X,A,B>;
		DoExpectation(zMock, X, NULL);
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
					((call.second.second != NULL) &&
					!((*tuple) == (*call.second.second)))) throw ExpectationException();
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

#endif

