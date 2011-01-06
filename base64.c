/*
 * Copyright (c) 2011 Pelle Johansson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "base64.h"

#include <err.h>

const char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char debase64[] =
	"\76" /* + */
	"\0\0\0"
	"\77\64\65\66\67\70\71\72\73\74\75" /* /, 0 - 9 */
	"\0\0\0"
	"\0" /* = */
	"\0\0\0"
	"\0\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31" /* A - Z */
	"\0\0\0\0\0\0"
	"\32\33\34\35\36\37\40\41\42\43\44\45\46\47\50\51\52\53\54\55\56\57\60\61\62\63"; /* a - z */

void
base64_int24(char *dst, int src) {
	dst[0] = base64[(unsigned)((src >> 18) & 0x3f)];
	dst[1] = base64[(unsigned)((src >> 12) & 0x3f)];
	dst[2] = base64[(unsigned)((src >> 6) & 0x3f)];
	dst[3] = base64[(unsigned)(src & 0x3f)];
}

int
debase64_int24(const char *src) {
	int res = 0;
	int i;
	const char *a;

	for (i = 0 ; i < 4 ; i++) {
		if (*a < '+' || *a > 'z' || (*a != 'A' && !debase64[*a - '+'])) {
			warnx("Invalid int: %.4s", src);
			return res;
		}
		res = (res << 6) | debase64[*a - '+'];
		if (i == 0 && res & 0x20) {
			/* High bit set => negative number. */
			res |= 0xFFFFFFC0;
		}
		a++;
	}
	return res;
}
