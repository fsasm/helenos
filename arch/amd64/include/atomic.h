/*
 * Copyright (C) 2001-2004 Jakub Jermar
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

#ifndef __amd64_ATOMIC_H__
#define __amd64_ATOMIC_H__

#include <arch/types.h>

typedef struct { volatile __u64 count; } atomic_t;

static inline void atomic_set(atomic_t *val, __u64 i)
{
	val->count = i;
}

static inline __u64 atomic_get(atomic_t *val)
{
	return val->count;
}

static inline void atomic_inc(atomic_t *val) {
#ifdef CONFIG_SMP
	__asm__ volatile ("lock incq %0\n" : "=m" (val->count));
#else
	__asm__ volatile ("incq %0\n" : "=m" (val->count));
#endif /* CONFIG_SMP */
}

static inline void atomic_dec(atomic_t *val) {
#ifdef CONFIG_SMP
	__asm__ volatile ("lock decq %0\n" : "=m" (val->count));
#else
	__asm__ volatile ("decq %0\n" : "=m" (val->count));
#endif /* CONFIG_SMP */
}

static inline count_t atomic_postinc(atomic_t *val) 
{
	count_t r;

	__asm__ volatile (
		"movq $1, %0\n"
		"lock xaddq %0, %1\n"
		: "=r" (r), "=m" (val->count)
	);

	return r;
}

static inline count_t atomic_postdec(atomic_t *val) 
{
	count_t r;
	
	__asm__ volatile (
		"movq $-1, %0\n"
		"lock xaddq %0, %1\n"
		: "=r" (r), "=m" (val->count)
	);
	
	return r;
}

#define atomic_preinc(val) (atomic_postinc(val)+1)
#define atomic_predec(val) (atomic_postdec(val)-1)

static inline __u64 test_and_set(atomic_t *val) {
	__u64 v;
	
	__asm__ volatile (
		"movq $1, %0\n"
		"xchgq %0, %1\n"
		: "=r" (v),"=m" (val->count)
	);
	
	return v;
}


extern void spinlock_arch(volatile int *val);

#endif
