/*
 * Copyright (c) 2001-2004 Jakub Jermar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
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

/** @addtogroup tasks
 * @{
 */

/**
 * @file
 * @brief Miscellaneous functions.
 */

#include <stdint.h>
#include "func.h"

void order(const uint64_t val, uint64_t *rv, char *suffix)
{
	if (val > 10000000000000000000ULL) {
		*rv = val / 1000000000000000000ULL;
		*suffix = 'Z';
	} else if (val > 1000000000000000000ULL) {
		*rv = val / 1000000000000000ULL;
		*suffix = 'E';
	} else if (val > 1000000000000000ULL) {
		*rv = val / 1000000000000ULL;
		*suffix = 'T';
	} else if (val > 1000000000000ULL) {
		*rv = val / 1000000000ULL;
		*suffix = 'G';
	} else if (val > 1000000000ULL) {
		*rv = val / 1000000ULL;
		*suffix = 'M';
	} else if (val > 1000000ULL) {
		*rv = val / 1000ULL;
		*suffix = 'k';
	} else {
		*rv = val;
		*suffix = ' ';
	}
}

/** @}
 */