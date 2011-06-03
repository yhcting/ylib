/*****************************************************************************
 *    Copyright (C) 2011 Younghyung Cho. <yhcting77@gmail.com>
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


#ifndef __YCRc_h__
#define __YCRc_h__

#include <stdint.h>

/**
 * @crc     : previous crc value
 * @buffer  : data pointer
 * @len     : number of bytes in the buffer.
 */
EXPORT uint16_t
ycrc16(uint16_t crc, const uint8_t* data, uint32_t len);


/**
 * See above 'ycrc16' for details.
 */
EXPORT uint32_t
ycrc32(uint32_t crc, const uint8_t* data, uint32_t len);


#endif /* __YCRc_h__ */
