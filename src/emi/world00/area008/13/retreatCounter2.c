#include "internal.h"
#include "game/counter_step.h"

/* @source 0x801F45A0
 * @behavior marks the local counter active and retreats it by 20.
 */
COUNTER_RETREAT(retreatCounter2, counter2,
                D_80149333)
