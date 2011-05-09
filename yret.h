/******************************************************************************
 * Abbrev
 *    YRxxx : yretxxx
 *****************************************************************************/


#ifndef __YREt_h__
#define __YREt_h__

#include "ydef.h"

#define DEFRETVAL(name, desc) name,
enum yret {
	/* Error should be minus value */
	YREVALStart = -0x7fffffff,
#include "yret_err.in"
	YREVALEnd,

	/* Don't change value of YROk! */
	YROk        = 0,

	/* Warning should be plus value */
	YRWVALStart = 0,
#include "yret_warn.in"
	YRWVALEnd
};
#undef DEFRETVAL


void
yretset(enum yret ret);


/***********************
 * Exported interface
 ***********************/

/*
 * get last warning or error
 */
EXPORT enum yret
yretget();

EXPORT const char*
yretstring(enum yret);


#endif /* __YREt_h__ */
