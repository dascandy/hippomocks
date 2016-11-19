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

#ifndef HIPPOMOCKS_OLDTUPLE_H
#define HIPPOMOCKS_OLDTUPLE_H

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
	virtual void printTo(std::ostream &os) const = 0;
};

template <typename A = NullType, typename B = NullType, typename C = NullType, typename D = NullType,
		  typename E = NullType, typename F = NullType, typename G = NullType, typename H = NullType,
		  typename I = NullType, typename J = NullType, typename K = NullType, typename L = NullType,
		  typename M = NullType, typename N = NullType, typename O = NullType, typename P = NullType>
class ref_tuple : public base_tuple
{
public:
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
	ref_tuple(A valueA = A(), B valueB = B(), C valueC = C(), D valueD = D(), E valueE = E(), F valueF = F(), G valueG = G(), H valueH = H(), I valueI = I(), J valueJ = J(), K valueK = K(), L valueL = L(), M valueM = M(), N valueN = N(), O valueO = O(), P valueP = P())
		  : a(valueA), b(valueB), c(valueC), d(valueD), e(valueE), f(valueF), g(valueG), h(valueH), i(valueI), j(valueJ), k(valueK), l(valueL), m(valueM), n(valueN), o(valueO), p(valueP)
	{}
	virtual void printTo(std::ostream &os) const
	{
	  os << "(";
	  printArg<A>::print(os, a, false);
	  printArg<B>::print(os, b, true);
	  printArg<C>::print(os, c, true);
	  printArg<D>::print(os, d, true);
	  printArg<E>::print(os, e, true);
	  printArg<F>::print(os, f, true);
	  printArg<G>::print(os, g, true);
	  printArg<H>::print(os, h, true);
	  printArg<I>::print(os, i, true);
	  printArg<J>::print(os, j, true);
	  printArg<K>::print(os, k, true);
	  printArg<L>::print(os, l, true);
	  printArg<M>::print(os, m, true);
	  printArg<N>::print(os, n, true);
	  printArg<O>::print(os, o, true);
	  printArg<P>::print(os, p, true);
	  os << ")";
	}
};

template <typename A = NullType, typename B = NullType, typename C = NullType, typename D = NullType,
		  typename E = NullType, typename F = NullType, typename G = NullType, typename H = NullType,
		  typename I = NullType, typename J = NullType, typename K = NullType, typename L = NullType,
		  typename M = NullType, typename N = NullType, typename O = NullType, typename P = NullType>
class ref_comparable_assignable_tuple : public base_tuple
{
public:
	virtual bool operator==(const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &bo) = 0;
	virtual void assign_from(ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &from) = 0;
  virtual void assign_to(ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &to) = 0;
};

template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L, typename M, typename N, typename O, typename P,
		  typename CA, typename CB, typename CC, typename CD, typename CE, typename CF, typename CG, typename CH,
		  typename CI, typename CJ, typename CK, typename CL, typename CM, typename CN, typename CO, typename CP>
class copy_tuple : public ref_comparable_assignable_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>
{
public:
	typename store_as<CA>::type a;
	typename store_as<CB>::type b;
	typename store_as<CC>::type c;
	typename store_as<CD>::type d;
	typename store_as<CE>::type e;
	typename store_as<CF>::type f;
	typename store_as<CG>::type g;
	typename store_as<CH>::type h;
	typename store_as<CI>::type i;
	typename store_as<CJ>::type j;
	typename store_as<CK>::type k;
	typename store_as<CL>::type l;
	typename store_as<CM>::type m;
	typename store_as<CN>::type n;
	typename store_as<CO>::type o;
	typename store_as<CP>::type p;
	copy_tuple(typename store_as<CA>::type valueA, typename store_as<CB>::type valueB,
		typename store_as<CC>::type valueC, typename store_as<CD>::type valueD,
		typename store_as<CE>::type valueE, typename store_as<CF>::type valueF,
		typename store_as<CG>::type valueG, typename store_as<CH>::type valueH,
		typename store_as<CI>::type valueI, typename store_as<CJ>::type valueJ,
		typename store_as<CK>::type valueK, typename store_as<CL>::type valueL,
		typename store_as<CM>::type valueM, typename store_as<CN>::type valueN,
		typename store_as<CO>::type valueO, typename store_as<CP>::type valueP)
		  : a(valueA), b(valueB), c(valueC), d(valueD), e(valueE), f(valueF), g(valueG), h(valueH), i(valueI), j(valueJ), k(valueK), l(valueL), m(valueM), n(valueN), o(valueO), p(valueP)
	{}
	bool operator==(const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &to)
	{
		return (comparer<A>::compare(a, to.a) &&
				comparer<B>::compare(b, to.b) &&
				comparer<C>::compare(c, to.c) &&
				comparer<D>::compare(d, to.d) &&
				comparer<E>::compare(e, to.e) &&
				comparer<F>::compare(f, to.f) &&
				comparer<G>::compare(g, to.g) &&
				comparer<H>::compare(h, to.h) &&
				comparer<I>::compare(i, to.i) &&
				comparer<J>::compare(j, to.j) &&
				comparer<K>::compare(k, to.k) &&
				comparer<L>::compare(l, to.l) &&
				comparer<M>::compare(m, to.m) &&
				comparer<N>::compare(n, to.n) &&
				comparer<O>::compare(o, to.o) &&
				comparer<P>::compare(p, to.p));
	}
	void assign_from(ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &from)
	{
		in_assign< typename store_as<CA>::type, A>(a, from.a);
		in_assign< typename store_as<CB>::type, B>(b, from.b);
		in_assign< typename store_as<CC>::type, C>(c, from.c);
		in_assign< typename store_as<CD>::type, D>(d, from.d);
		in_assign< typename store_as<CE>::type, E>(e, from.e);
		in_assign< typename store_as<CF>::type, F>(f, from.f);
		in_assign< typename store_as<CG>::type, G>(g, from.g);
		in_assign< typename store_as<CH>::type, H>(h, from.h);
		in_assign< typename store_as<CI>::type, I>(i, from.i);
		in_assign< typename store_as<CJ>::type, J>(j, from.j);
		in_assign< typename store_as<CK>::type, K>(k, from.k);
		in_assign< typename store_as<CL>::type, L>(l, from.l);
		in_assign< typename store_as<CM>::type, M>(m, from.m);
		in_assign< typename store_as<CN>::type, N>(n, from.n);
		in_assign< typename store_as<CO>::type, O>(o, from.o);
		in_assign< typename store_as<CP>::type, P>(p, from.p);
	}
	void assign_to(ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &to)
	{
		out_assign< typename store_as<CA>::type, A>(a, to.a);
		out_assign< typename store_as<CB>::type, B>(b, to.b);
		out_assign< typename store_as<CC>::type, C>(c, to.c);
		out_assign< typename store_as<CD>::type, D>(d, to.d);
		out_assign< typename store_as<CE>::type, E>(e, to.e);
		out_assign< typename store_as<CF>::type, F>(f, to.f);
		out_assign< typename store_as<CG>::type, G>(g, to.g);
		out_assign< typename store_as<CH>::type, H>(h, to.h);
		out_assign< typename store_as<CI>::type, I>(i, to.i);
		out_assign< typename store_as<CJ>::type, J>(j, to.j);
		out_assign< typename store_as<CK>::type, K>(k, to.k);
		out_assign< typename store_as<CL>::type, L>(l, to.l);
		out_assign< typename store_as<CM>::type, M>(m, to.m);
		out_assign< typename store_as<CN>::type, N>(n, to.n);
		out_assign< typename store_as<CO>::type, O>(o, to.o);
		out_assign< typename store_as<CP>::type, P>(p, to.p);
	}
	virtual void printTo(std::ostream &os) const
	{
		os << "(";
		printArg<typename store_as<CA>::type>::print(os, a, false);
		printArg<typename store_as<CB>::type>::print(os, b, true);
		printArg<typename store_as<CC>::type>::print(os, c, true);
		printArg<typename store_as<CD>::type>::print(os, d, true);
		printArg<typename store_as<CE>::type>::print(os, e, true);
		printArg<typename store_as<CF>::type>::print(os, f, true);
		printArg<typename store_as<CG>::type>::print(os, g, true);
		printArg<typename store_as<CH>::type>::print(os, h, true);
		printArg<typename store_as<CI>::type>::print(os, i, true);
		printArg<typename store_as<CJ>::type>::print(os, j, true);
		printArg<typename store_as<CK>::type>::print(os, k, true);
		printArg<typename store_as<CL>::type>::print(os, l, true);
		printArg<typename store_as<CM>::type>::print(os, m, true);
		printArg<typename store_as<CN>::type>::print(os, n, true);
		printArg<typename store_as<CO>::type>::print(os, o, true);
		printArg<typename store_as<CP>::type>::print(os, p, true);
		os << ")";
	}
};

// Do() function wrapping
class VirtualDestructable { public: virtual ~VirtualDestructable() {} };

template <typename Y>
class TupleInvocable : public VirtualDestructable
{
public:
	virtual Y operator()(const base_tuple &tupl) = 0;
};

template <typename Y,
		  typename A = NullType, typename B = NullType, typename C = NullType, typename D = NullType,
		  typename E = NullType, typename F = NullType, typename G = NullType, typename H = NullType,
		  typename I = NullType, typename J = NullType, typename K = NullType, typename L = NullType,
		  typename M = NullType, typename N = NullType, typename O = NullType, typename P = NullType>
class Invocable : public TupleInvocable<Y>
{
public:
	virtual Y operator()(A a = A(), B b = B(), C c = C(), D d = D(), E e = E(), F f = F(), G g = G(), H h = H(), I i = I(), J j = J(), K k = K(), L l = L(), M m = M(), N n = N(), O o = O(), P p = P()) = 0;
	virtual Y operator()(const base_tuple &tupl) {
		const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &rTupl = reinterpret_cast<const ref_tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &>(tupl);
		return (*this)(rTupl.a, rTupl.b, rTupl.c, rTupl.d, rTupl.e, rTupl.f, rTupl.g, rTupl.h,
			rTupl.i, rTupl.j, rTupl.k, rTupl.l, rTupl.m, rTupl.n, rTupl.o, rTupl.p);
	}
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N, typename O, typename P>
class DoWrapper : public Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p)
	{
		return t(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N, typename O>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> : public Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, NullType)
	{
		return t(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g,h,i,j,k,l,m,n);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g,h,i,j,k,l,m);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g,h,i,j,k,l);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F,G,H,I,J,K> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g,h,i,j,k);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I,J,K>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I, typename J>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F,G,H,I,J> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g,h,i,j);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I,J>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H,
		  typename I>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType>  : public Invocable<Y,A,B,C,D,E,F,G,H,I>{
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g,h,i);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G, typename H>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F,G,H> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g,h);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F, typename G>
class DoWrapper<T,Y,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F,G> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g);
	}
	using Invocable<Y,A,B,C,D,E,F,G>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E, typename F>
class DoWrapper<T,Y,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f);
	}
	using Invocable<Y,A,B,C,D,E,F>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D,
		  typename E>
class DoWrapper<T,Y,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, B b, C c, D d, E e, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e);
	}
	using Invocable<Y,A,B,C,D,E>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D>
class DoWrapper<T,Y,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, B b, C c, D d, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d);
	}
	using Invocable<Y,A,B,C,D>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C>
class DoWrapper<T,Y,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, B b, C c, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c);
	}
	using Invocable<Y,A,B,C>::operator();
};
template <typename T, typename Y, typename A, typename B>
class DoWrapper<T,Y,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, B b, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b);
	}
	using Invocable<Y,A,B>::operator();
};
template <typename T, typename Y, typename A>
class DoWrapper<T,Y,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(A a, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a);
	}
	using Invocable<Y,A>::operator();
};
template <typename T, typename Y>
class DoWrapper<T,Y,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y> {
	T t;
public:
	DoWrapper(T templ) : t(templ) {}
	virtual Y operator()(NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t();
	}
	using Invocable<Y>::operator();
};

#endif


