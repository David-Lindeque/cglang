#ifndef __UTILSH
#define __UTILSH

#ifdef WINDOWS

#include <string.h>
#include <stdio.h>
#include <locale.h>

namespace cglang
{
	inline int wstricmp(const wchar_t *str1, const wchar_t *str2) {
		return ::_wcsicmp(str1, str2);
	}

	inline int stricmp(const char *str1, const char *str2) {
		return ::_stricmp(str1, str2);
	}

	inline FILE* fopen(const char *fn, const char *mode) {
		FILE *f;
		if (!::fopen_s(&f, fn, mode)) return f;
		else return NULL;
	}
}

#else

#include <wchar.h>

namespace cglang
{
	inline int wstrcmp(const wchar_t *str1, const wchar_t *str2) {
		return ::wcscasecmp(str1, str2);
	}

	inline int stricmp(const char* str1, const char *str2) {
		return ::strcasecmp(str1, str2);
	}

	inline FILE* fopen(const char *fn, const char *mode) {
		return ::fopen(fn, mode);
	}
}
#endif

#endif