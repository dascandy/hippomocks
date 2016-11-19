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

#ifndef HIPPOMOCKS_EXCEPTIONS_H
#define HIPPOMOCKS_EXCEPTIONS_H

#if defined(__GNUC__) && !defined(__EXCEPTIONS)
#define HM_NO_EXCEPTIONS
class X{};
#define BASE_EXCEPTION X
#else
#ifndef BASE_EXCEPTION
#define BASE_EXCEPTION std::exception
#include <exception>
#endif
#endif

#ifdef HM_NO_EXCEPTIONS
#define RAISEEXCEPTION(e) 			{ std::string err = e.what(); DEBUGBREAK(e); printf("Mock error found - Fatal due to no exception support:\n"); \
	printf("%s\n", err.c_str()); \
	abort(); exit(-1); }
#define RAISELATENTEXCEPTION(e) 	{ std::string err = e.what(); DEBUGBREAK(e); printf("Mock error found - Fatal due to no exception support:\n"); \
	printf("%s\n", err.c_str()); \
	abort(); exit(-1); }
#else

//Type-safe exception wrapping
class ExceptionHolder
{
public:
	virtual ~ExceptionHolder() {}
	virtual void rethrow() = 0;
	template <typename T>
	static ExceptionHolder *Create(T ex);
};

template <class T>
class ExceptionWrapper : public ExceptionHolder {
	T exception;
public:
	ExceptionWrapper(T ex) : exception(ex) {}
	void rethrow() { throw exception; }
};

template <typename T>
ExceptionHolder *ExceptionHolder::Create(T ex)
{
	return new ExceptionWrapper<T>(ex);
}

#define RAISEEXCEPTION(e)			{ DEBUGBREAK(e); throw e; }
#define RAISELATENTEXCEPTION(e)		{ DEBUGBREAK(e); if (std::uncaught_exception()) \
	MockRepoInstanceHolder<0>::instance->SetLatentException(ExceptionHolder::Create(e)); \
	else throw e; }
#endif

class BaseException
#ifndef HM_NO_EXCEPTIONS
	: public BASE_EXCEPTION
#endif
{
public:
	~BaseException() throw() {}
	const char *what() const throw() { return txt.c_str(); }
protected:
	std::string txt;
};

// exception types
class ExpectationException : public BaseException {
public:
	ExpectationException(MockRepository *repo, const base_tuple *tuple, const char *funcName)
	{
		std::stringstream text;
		text << "Function ";
		text << funcName;
		if (tuple)
			tuple->printTo(text);
		else
			text << "(...)";
		text << " called with mismatching expectation!" << std::endl;
		text << *repo;
		txt = text.str();
	}
};

#ifdef LINUX_TARGET
#include <execinfo.h>
#endif

class NotImplementedException : public BaseException {
public:
	NotImplementedException(MockRepository *repo)
	{
		std::stringstream text;
		text << "Function called without expectation!" << std::endl;
		text << *repo;

#ifdef LINUX_TARGET
		void* stacktrace[256];
		size_t size = backtrace( stacktrace, sizeof(stacktrace) );
		if( size > 0 )
		{
			text << "Stackdump:" << std::endl;
			char **symbols = backtrace_symbols( stacktrace, size );
			for( size_t i = 0; i < size; i = i + 1 )
			{
				text << symbols[i] << std::endl;
			}
			free( symbols );
		}
#endif

		txt = text.str();
	}
};

class CallMissingException : public BaseException {
public:
	CallMissingException(MockRepository *repo)
	{
		std::stringstream text;
		text << "Function with expectation not called!" << std::endl;
		text << *repo;
		txt = text.str();
	}
};

class ZombieMockException : public BaseException {
public:
	ZombieMockException(MockRepository *repo)
	{
		std::stringstream text;
		text << "Function called on mock that has already been destroyed!" << std::endl;
		text << *repo;

#ifdef LINUX_TARGET
	void* stacktrace[256];
	size_t size = backtrace( stacktrace, sizeof(stacktrace) );
	if( size > 0 )
	{
	  text << "Stackdump:" << std::endl;
	  char **symbols = backtrace_symbols( stacktrace, size );
	  for( size_t i = 0; i < size; i = i + 1 )
	  {
		text << symbols[i] << std::endl;
	  }
	  free( symbols );
	}
#endif

		txt = text.str();
	}
};

class NoResultSetUpException : public BaseException {
public:
	NoResultSetUpException(MockRepository *repo, const base_tuple *tuple, const char *funcName)
	{
		std::stringstream text;
		text << "No result set up on call to ";
		text << funcName;
		if (tuple)
			tuple->printTo(text);
		else
			text << "(...)";
		text << std::endl;
		text << *repo;

#ifdef LINUX_TARGET
	void* stacktrace[256];
	size_t size = backtrace( stacktrace, sizeof(stacktrace) );
	if( size > 0 )
	{
	  text << "Stackdump:" << std::endl;
	  char **symbols = backtrace_symbols( stacktrace, size );
	  for( size_t i = 0; i < size; i = i + 1 )
	  {
		text << symbols[i] << std::endl;
	  }
	  free( symbols );
	}
#endif

		txt = text.str();
	}
};

#endif


