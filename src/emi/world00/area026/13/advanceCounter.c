#include "internal.h"
#include "game/counter_step.h"

/* @source 0x801F3258
 * @behavior marks the local counter active and advances it by 20.
 */
COUNTER_ADVANCE(advanceCounter, counter,
                D_80149333)
