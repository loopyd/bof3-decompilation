#ifndef BASE_COMPILER_H
#define BASE_COMPILER_H

#include "base/types.h"

#if defined(__GNUC__)
#define NO_SIBLING_CALLS __attribute__((optimize("no-optimize-sibling-calls")))
#else
#define NO_SIBLING_CALLS
#endif

#endif
