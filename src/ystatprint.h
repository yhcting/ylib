/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014
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

/**
 * @file ystatprint.h
 * @brief Header to printing ascii bar graph with statistics data.
 */

#ifndef __YSTATPRINt_h__
#define __YSTATPRINt_h__

#include "ydef.h"

/**
 * Print ascii bar graph like below.
 * <PRE>
 *
 *  ------------------------- &lt;max&gt;
 *              *
 *  *           * *
 *  *           * *
 *  *         * * *
 *  * * *   * * * * *
 *  * * * * * * * * * * * * *
 *  ------------------------- &lt;min&gt;
 *            ^
 *            mean
 * </PRE>
 *
 * @param fd File to write.
 * @param vs Array of values
 * @param vsz Size of value array
 * @param height Height of longest bar
 * @param idxs Index array to add comment at.
 *             This SHOULD be incrementally sorted array.
 * @param cmts Array of comment string.
 * @param isz Array size of {@code idxs} and {@code cmts}
 * @param isp Inter-space. # of spaces between bar.
 * @param barc Ascii character to represent bar.
 * @return 0 for success otherwise -(error number).
 */
YYEXPORT int
ystpr_bargraph(int fd,
	       const double *vs,
	       uint32_t vsz,
	       uint32_t height,
	       const uint32_t *idxs,
	       const char * const *cmts,
	       uint32_t isz,
	       uint32_t isp,
	       char barc);



/**
 * [UNDER-IMPLEMENTATION]
 *
 * Print ascii distribution bar graph like below
 *
 *  Samples: xx
 *  Mean: xxxx
 *  Standard Deviation: xxx
 * <PRE>
 *  ------------------------- &lt;max&gt;
 *              *
 *  *           * *
 *  *           * *
 *  *         * * *
 *  * * *   * * * * *
 *  * * * * * * * * * * * * *
 *  ------------------------- &lt;min&gt;
 *  ^^       ^       ^      ^
 *   -2s     -s      u      s
 *  &lt;min&gt;                   &lt;max&gt;
 * </PRE>
 *
 * @param fd File to write.
 * @param vs Array of values
 * @param vsz Size of value array
 * @param w Graph width
 * @param h Graph height
 * @param isp Inter-space. # of spaces between bar.
 * @param barc Ascii character to represent bar.
 * @return 0 for success otherwise -(error number).
 */
YYEXPORT int
ystpr_distgraph(int fd,
		const double *vs,
		uint32_t vsz,
		uint32_t w,
		uint32_t h,
		uint32_t isp,
		char barc);

#endif /* __YSTATPRINt_h__ */
