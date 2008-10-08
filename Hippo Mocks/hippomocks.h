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

extern base_mock *mockUnderConstruction;

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
	static void __stdcall d0(T *obj) { ((mock<T> *)(mockUnderConstruction))->setDestructor(0); };
	static void __stdcall d1(T *obj) { ((mock<T> *)(mockUnderConstruction))->setDestructor(1); };
	static void __stdcall d2(T *obj) { ((mock<T> *)(mockUnderConstruction))->setDestructor(2); };
	static void __stdcall d3(T *obj) { ((mock<T> *)(mockUnderConstruction))->setDestructor(3); };
	static void __stdcall d4(T *obj) { ((mock<T> *)(mockUnderConstruction))->setDestructor(4); };
	static void __stdcall d5(T *obj) { ((mock<T> *)(mockUnderConstruction))->setDestructor(5); };
	static void __stdcall d6(T *obj) { ((mock<T> *)(mockUnderConstruction))->setDestructor(6); };
	static void __stdcall d7(T *obj) { ((mock<T> *)(mockUnderConstruction))->setDestructor(7); };
	static void __stdcall d8(T *obj) { ((mock<T> *)(mockUnderConstruction))->setDestructor(8); };
	static void __stdcall d9(T *obj) { ((mock<T> *)(mockUnderConstruction))->setDestructor(9); };
	void setDestructor(int index) {
		funcs[index] = (void (*)())destr; vdestructor = index;
	}
	void base_func(int which) 
	{
		repo->DoExpectation(this, which);
	};
	int last_call;
	int vdestructor;
	static void __stdcall destr() {}
	MockRepository *repo;
public:
	mock(MockRepository *repo) 
		: repo(repo),
		vdestructor(-1)
	{
		funcs[0] = (void (*)())d0;
		funcs[1] = (void (*)())d1;
		funcs[2] = (void (*)())d2;
		funcs[3] = (void (*)())d3;
		funcs[4] = (void (*)())d4;
		funcs[5] = (void (*)())d5;
		funcs[6] = (void (*)())d6;
		funcs[7] = (void (*)())d7;
		funcs[8] = (void (*)())d8;
		funcs[9] = (void (*)())d9;
		memset(base.remaining, 0, sizeof(base.remaining));
		base.vft = (void *)&funcs;
		mockUnderConstruction = this;
		T *tp = reinterpret_cast<T *>(this);
		tp->~T();

		mockUnderConstruction = NULL;
		if (vdestructor != 0) funcs[0] = (void (*)())f0;
		if (vdestructor != 1) funcs[1] = (void (*)())f1;
		if (vdestructor != 2) funcs[2] = (void (*)())f2;
		if (vdestructor != 3) funcs[3] = (void (*)())f3;
		if (vdestructor != 4) funcs[4] = (void (*)())f4;
		if (vdestructor != 5) funcs[5] = (void (*)())f5;
		if (vdestructor != 6) funcs[6] = (void (*)())f6;
		if (vdestructor != 7) funcs[7] = (void (*)())f7;
		if (vdestructor != 8) funcs[8] = (void (*)())f8;
		if (vdestructor != 9) funcs[9] = (void (*)())f9;
	}
};

class MockRepository {
private:
	std::list<base_mock *> mocks;
	std::list<std::pair<base_mock *, int> > expectations;
	enum { Record, Playback, Verified } state;
public:
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

