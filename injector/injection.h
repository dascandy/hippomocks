/*
A template for code injection.

Note that we don't use a constructor/destructor here, because the
injected function will need access to this object. In general your
code will look like this:

Injection<FnType> injection;

int replacement()
{
  // ...
	return injection.fnOriginal();
}

void do()
{
  injection.install(something, replacement);
	something();
	injection.uninstall();
}

*/

#include "injector.h"

// T is expected to be a function
template <typename T> class Injection
{
public:
	T fnOriginal;

	Injection(): fnOriginal(NULL)
	{
	}

	~Injection()
	{
		uninstall();
	}

	bool isInstalled() const
	{
		return fnOriginal != NULL;
	}

#ifdef _WIN32
	/* This variation is handy if you are intercepting DLL calls.
	 */
	bool install(HMODULE hModule, UINT_PTR offset, T fnReplacement)
	{
		if ((offset == NULL) || (hModule == NULL))
		{
			return false;
		}
		// In Win32 and Win64, a module handle is the base pointer for the
		// methods it exports. This allows function pointers to be passed
		// and translated between processes.
		fnOriginal = (T)inject((char*)hModule + offset, fnReplacement);
		return (fnOriginal != NULL);
	}
#endif

	bool install(void* fnOriginalFunc, T fnReplacement)
	{
		if (fnOriginalFunc == NULL)
		{
			return false;
		}
		fnOriginal = (T)inject(fnOriginalFunc, fnReplacement);
		return (fnOriginal != NULL);
	}

	void uninstall()
	{
		if (fnOriginal != NULL)
		{
			uninject((PBYTE)fnOriginal);
			fnOriginal = NULL;
		}
	}

	/*TODO: Nice overload to allow something like :
	  f = Injection<F>;
		f.install(..., g);
		g() { ... f(); ... };
	*/
};
