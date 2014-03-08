/******************************************************************************
 *    Copyright (C) 2011, 2012, 2013, 2014
 *    Younghyung Cho. <yhcting77@gmail.com>
 *
 *    This file is part of ylib
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License
 *    (<http://www.gnu.org/licenses/lgpl.html>) for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.	If not, see <http://www.gnu.org/licenses/>.
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
