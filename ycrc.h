#ifndef __YCRc_h__
#define __YCRc_h__

/**
 * Assumption : sizeof(unsigned short) == 16
 * @crc     : prevvious crc value
 * @buffer  : data pointer
 * @len     : number of bytes in the buffer.
 */
EXPORT unsigned short
ycrc16(unsigned short crc, const unsigned char* data, unsigned int len);


/**
 * Assumption : sizeof(unsigned int) == 32
 * See above 'ycrc16' for details.
 */
EXPORT unsigned int
ycrc32(unsigned int crc, const unsigned char* data, unsigned int len);


#endif /* __YCRc_h__ */
