#include "bof3/world/area00813_internal.h"
#include "game/counter_step.h"

/* @source 0x801F45A0
 * @behavior marks the local counter active and retreats it by 20.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
COUNTER_RETREAT(retreatCounter2, counter2,
                D_80149333)
