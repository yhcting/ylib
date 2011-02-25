#ifndef __COMMOn_h__
#define __COMMOn_h__

#include "ydef.h"

/*=======================================
 * Macros
 *=======================================*/

#ifndef abs
#	define abs(x) (((x)>0)?(x):-(x))
#endif

#ifndef swap
#	define swap(x,y,tmp) do { tmp=x;x=y;y=tmp; } while (0)
#endif

#ifndef min
#	define min(x,y) (((x)<(y))?x:y)
#endif

#ifndef max
#	define max(x,y) (((x)<(y))?y:x)
#endif

#endif /* __COMMOn_h__ */
