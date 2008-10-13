#ifndef HIPPOMOCKS_H
#define HIPPOMOCKS_H

#include <list>
#include <map>

class base_mock {};

class MockRepository;

class ExpectationException : public std::exception {
public:
	const char *what() const { return "Expectation was violated!"; }
};

class NullType {};

template <typename T> struct size { enum { value = ((4 > sizeof(T)) ? sizeof(T) : 4) }; };
template <> struct size<NullType> { enum { value = 0 }; };

template <typename A = NullType, typename B = NullType, typename C = NullType, typename D = NullType, 
		  typename E = NullType, typename F = NullType, typename G = NullType, typename H = NullType, 
		  typename I = NullType, typename J = NullType, typename K = NullType, typename L = NullType, 
		  typename M = NullType, typename N = NullType, typename O = NullType, typename P = NullType>
struct sizelist { enum { value = size<A>::value + size<B>::value + size<C>::value + size<D>::value + 
								 size<E>::value + size<F>::value + size<G>::value + size<H>::value + 
								 size<I>::value + size<J>::value + size<K>::value + size<L>::value + 
								 size<M>::value + size<N>::value + size<O>::value + size<P>::value }; };

template <class T>
class mock : public base_mock {
	struct {
		void *vft;
		char remaining[sizeof(T)];
	} base;
	void (*funcs[10])();
	static void __thiscall f0(T *obj) { reinterpret_cast<mock<T> *>(obj)->base_func(0); };
	static void __thiscall f1(T *obj) { reinterpret_cast<mock<T> *>(obj)->base_func(1); };
	static void __thiscall f2(T *obj) { reinterpret_cast<mock<T> *>(obj)->base_func(2); };
	static void __thiscall f3(T *obj) { reinterpret_cast<mock<T> *>(obj)->base_func(3); };
	static void __thiscall f4(T *obj) { reinterpret_cast<mock<T> *>(obj)->base_func(4); };
	static void __thiscall f5(T *obj) { reinterpret_cast<mock<T> *>(obj)->base_func(5); };
	static void __thiscall f6(T *obj) { reinterpret_cast<mock<T> *>(obj)->base_func(6); };
	static void __thiscall f7(T *obj) { reinterpret_cast<mock<T> *>(obj)->base_func(7); };
	static void __thiscall f8(T *obj) { reinterpret_cast<mock<T> *>(obj)->base_func(8); };
	static void __thiscall f9(T *obj) { reinterpret_cast<mock<T> *>(obj)->base_func(9); };
	void base_func(int which) 
	{
		if (vdestructor == -1) 
		{
			funcs[which] = (void (*)())destr;
		}
		else
		{
			repo->DoExpectation(this, which);
		}

		if (nextFuncSize != 0) 
		{
			funcSizes[which] = nextFuncSize;
			nextFuncSize = 0;
		}
	};
	int last_call;
	int vdestructor;
	int nextFuncSize;
	int funcSizes[10];
	static void __stdcall destr() {}
	MockRepository *repo;
public:
	void setNextFuncSize(int size) 
	{
		nextFuncSize = size; 
	}
	mock(MockRepository *repo) 
		: repo(repo)
	{
		funcs[0] = (void (*)())f0;
		funcs[1] = (void (*)())f1;
		funcs[2] = (void (*)())f2;
		funcs[3] = (void (*)())f3;
		funcs[4] = (void (*)())f4;
		funcs[5] = (void (*)())f5;
		funcs[6] = (void (*)())f6;
		funcs[7] = (void (*)())f7;
		funcs[8] = (void (*)())f8;
		funcs[9] = (void (*)())f9;
		//TODO: find some way of constructing the interface type without 
		// explicitly "implementing" its functions
		// can't use normal constructor method because of pure virtuals
		// can't inherit because of pure virtuals
		// if you don't construct it, it fails because it might contain
		// objects which are then not constructed.
		memset(base.remaining, 0, sizeof(base.remaining));
		base.vft = (void *)&funcs;
		/*TODO: this code needs some work - detecting which is the destructor 
		 * works, the ret in the destructor doesn't
		T *tp = reinterpret_cast<T *>(this);
		vdestructor == -1;
		tp->~T();
		vdestructor == -2;
		*/
	}
};

class MockRepository {
private:
	std::list<base_mock *> mocks;
	std::list<std::pair<base_mock *, int> > expectations;
	enum { Record, Playback, Verified } state;
public:
	// for next year
#ifdef CPP0X
	template <typename Y, typename Z, typename A...>
	void RegisterExpectation(Z *mock, Y (Z::*func)(A...), A... a) {
		reinterpret_cast<mock<Z> *>(mock)->setNextFuncSize(sizelist<A...>::value);
		(mock->*func)(a...);
	}
#else
	template <typename Y, typename Z>
	void RegisterExpectation(Z *mck, Y (Z::*func)()) {
		reinterpret_cast<mock<Z> *>(mck)->setNextFuncSize(sizelist<>::value);
		(mck->*func)();
	}
	template <typename Y, typename Z, typename A>
	void RegisterExpectation(Z *mck, Y (Z::*func)(A), A a) {
		reinterpret_cast<mock<Z> *>(mck)->setNextFuncSize(sizelist<A>::value);
		(mck->*func)(a);
	}
	template <typename Y, typename Z, typename A, typename B>
	void RegisterExpectation(Z *mck, Y (Z::*func)(A, B), A a, B b) {
		reinterpret_cast<mock<Z> *>(mck)->setNextFuncSize(sizelist<A, B>::value);
		(mck->*func)(a, b);
	}
#endif
	void DoExpectation(base_mock *mock, int funcno) 
	{
		switch(state)
		{
		case Record: 
			expectations.push_back(std::make_pair(mock, funcno)); 
			break;
		case Playback: 
			{
				if (expectations.size() == 0)
					throw ExpectationException();

				std::pair<base_mock *, int> call = expectations.front(); 
				expectations.pop_front(); 
				if (mock != call.first || funcno != call.second) throw ExpectationException();
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

