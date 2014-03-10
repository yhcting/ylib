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

#ifndef __YSTATPRINt_h__
#define __YSTATPRINt_h__

/**
 * Print ascii bar graph like below.
 *
 *  ------------------------- <max>
 *              *
 *  *           * *
 *  *           * *
 *  *         * * *
 *  * * *   * * * * *
 *  * * * * * * * * * * * * *
 *  ------------------------- <min>
 *            ^
 *            mean
 *
 * @fd        file to write.
 * @vs        array of values 
 * @vsz       size of value array
 * @height    height of longest bar
 * @idxs      index array to add comment at.
 *            This SHOULD be incrementally sorted array.
 * @s         array of comment string
 * @isz       array size of 'idxs' and 's'
 * @isp       inter-space. # of spaces between bar.
 * @barc      ascii character to represent bar.
 * @return    0 for success otherwise error number.
 */
EXPORT int
ystpr_bargraph(int                 fd,
	       const double       *vs,
	       unsigned int        vsz,
	       unsigned int        height,
	       const unsigned int *idxs,
	       const char * const *cmts,
	       unsigned int        isz,
	       unsigned int        isp,
	       char                barc);



/**
 * [UNDER-IMPLEMENTATION]
 *
 * Print ascii distribution bar graph like below
 *
 *  Samples: xx
 *  Mean: xxxx
 *  Standard Deviation: xxx
 *  ------------------------- <max>
 *              *
 *  *           * *
 *  *           * *
 *  *         * * *
 *  * * *   * * * * *
 *  * * * * * * * * * * * * *
 *  ------------------------- <min>
 *  ^^       ^       ^      ^
 *   -2s     -s      u      s
 *  <min>                   <max>
 *
 * @fd        file to write.
 * @vs        array of values 
 * @vsz       size of value array
 * @w         graph width
 * @h         graph height
 * @isp       inter-space. # of spaces between bar.
 * @barc      ascii character to represent bar.
 * @return    0 for success otherwise error number.
 */
EXPORT int
ystpr_distgraph(int           fd,
		const double *vs,
		unsigned int  vsz,
		unsigned int  w,
		unsigned int  h,
		unsigned int  isp,
		char          barc);

#endif /* __YSTATPRINt_h__ */
