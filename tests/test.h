/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014, 2015, 2016
 * Younghyung Cho. <yhcting77@gmail.com>
 * All rights reserved.
 *
 * This file is part of ylib
 *
 * This program is licensed under the FreeBSD license
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of the FreeBSD Project.
 *****************************************************************************/

#ifndef __TESt_h__
#define __TESt_h__

#include "common.h"

EXPORT int dmem_count(void);
EXPORT void dregister_tstfn(void (*fn)(void), const char *mod);
EXPORT void dunregister_tstfn(void (*fn)(void), const char *mod);
EXPORT void dregister_clearfn(void (*fn)(void), const char *mod);
EXPORT void dunregister_clearfn(void (*fn)(void), const char *mod);


#  define TESTFN(name)							\
	static void __tst_register_test_##name(void) __attribute__ ((constructor)); \
	static void __tst_register_test_##name(void) {			\
		dregister_tstfn(&test_##name, #name);			\
	}								\
	static void __tst_unregister_test_##name(void) __attribute__ ((destructor)); \
	static void __tst_unregister_test_##name(void) {		\
		dunregister_tstfn(&test_##name, #name);			\
	}

#  define CLEARFN(name)							\
	static void __tst_register_clear_##name(void) __attribute__ ((constructor)); \
	static void __tst_register_clear_##name(void) {			\
		dregister_clearfn(&clear_##name, #name);			\
	}

#endif /* __TESt_h__ */
