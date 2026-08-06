#include "internal.h"
#include "game/counter_step.h"

/* @source 0x801F3230
 * @behavior marks the local counter active and retreats it by 20.
 */
COUNTER_RETREAT(retreatCounter, counter,
                D_80149333)
