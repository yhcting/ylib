#ifndef _COMMOn_h_
#define _COMMOn_h_

#include "ydef.h"

/*=======================================
 * Macros
 *=======================================*/

#ifdef __YUNITTEST__
#   include <assert.h>
#endif /* __YUNITTEST__ */


#ifndef abs
#	define abs(x) (((x)>0)?(x):-(x))
#endif

#ifndef swap
#	define swap(x,y,tmp) do { tmp=x;x=y;y=tmp; } while(0)
#endif

#ifndef min
#	define min(x,y) (((x)<(y))?x:y)
#endif

#ifndef max
#	define max(x,y) (((x)<(y))?y:x)
#endif

#endif /* _COMMOn_h_ */
