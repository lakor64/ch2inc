#pragma once

#define BR_EOF (-1)

#define BR_ERROR_MAKE(class,sub) ((class) | (sub))
#define BRE_UNKNOWN			BR_ERROR_MAKE(BR_ECLASS_MISC,1)
#define BRE_FAIL			BR_ERROR_MAKE(BR_ECLASS_MISC,2)
