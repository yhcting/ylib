#ifndef _YCRc_h_
#define _YCRc_h_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Assumption : sizeof(unsigned short) == 16
 * @crc     : prevvious crc value
 * @buffer  : data pointer
 * @len     : number of bytes in the buffer.
 */
unsigned short ycrc16(unsigned short crc, const char* data, unsigned int len);


/**
 * Assumption : sizeof(unsigned int) == 32
 * See above 'ycrc16' for details.
 * (-- Not implemented yet! --)
 */
 unsigned int ycrc32(unsigned int crc, const char* data, unsigned int len);


#ifdef __cplusplus
}
#endif

#endif /* _YCRc_h_ */
