#pragma once

#if defined(DLL_EXPORTS)
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif