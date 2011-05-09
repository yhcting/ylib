/*
 * Memory pool is tested at ylisp and writer.
 * So, there is no specific test file....
 */

#ifndef __YMEMPOOl_h__
#define __YMEMPOOl_h__

#include "yret.h"

/*
 * mp : Memory Pool
 */
struct ymp;

/*
 * size of pool will be expanded if needed.
 * this is just 'initial' size
 * grpsz  : elem group size size in pool (number of element)
 * elemsz : element size (in bytes)
 */
struct ymp*
ymp_create(int grpsz, int elemsz);

void
ymp_destroy(struct ymp* mp);

/*
 * get one block from pool.
 */
void*
ymp_get(struct ymp* mp);

/*
 * return block to pool.
 */
void
ymp_put(struct ymp* mp, void* block);

/*
 * interrupt shrinking.
 * this is NOT implemented yet!!
 */
enum yret
ymp_stop_shrink(struct ymp*);

/*
 * shrink memory pool.
 */
enum yret
ymp_shrink(struct ymp*);

/*
 * return number of element size
 */
int
ymp_sz(struct ymp* mp);

#endif /* __YMEMPOOl_h__ */
