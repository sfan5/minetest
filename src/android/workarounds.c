/*
Minetest
Copyright (C) 2013 sfan5 <sfan5@live.de>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdlib.h>
#include <wchar.h>

// This file contains workarounds for Android because the libc defines things it doesn't even have

int wctomb(char *s, wchar_t wc)
{
	return wcrtomb(s,wc,NULL);
}

int mbtowc(wchar_t *pwc, const char *s, size_t n)
{
	return mbrtowc(pwc, s, n, NULL);
}

unsigned long ___runetype(__ct_rune_t rune)
{
	(void)rune;
	return 0L;
}

long double strtold(const char* str, char** endptr)
{
	return strtod(str, endptr);
}
