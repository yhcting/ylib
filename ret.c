/******************************************************************************
 *
 * Multi-threading is not considered yet!
 *
 *****************************************************************************/


#include "yret.h"



static enum yret _last_ret = YROk; /* last error of this ylibrary */


#define DEFRETVAL(name, desc) desc,
static const char* _errdesc[] = {
#include "yret_err.in"
};

static const char* _warndesc[] = {
#include "yret_warn.in"
};
#undef DEFRETVAL


void
yretset(enum yret err) { _last_ret = err; }

enum yret
yretget() { return _last_ret; }

const char*
yretstring(enum yret r) {
	if (YROk == r) {
		return "Ok";
	} else if (YREVALStart < r && YREVALEnd > r) {
		yassert(r < 0);
		return _errdesc[r - YREVALStart - 1];
	} else if (YRWVALStart < r && YRWVALEnd > r) {
		yassert(r > 0);
		return _warndesc[r- YRWVALStart - 1];
	} else {
		/* Invalid argument */
		yassert(0);
		return NULL;
	}
}

