#include "bof3/world/area02613_internal.h"
#include "game/counter_step.h"

/* @source 0x801F3230
 * @behavior marks the local counter active and retreats it by 20.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
COUNTER_RETREAT(retreatCounter, counter,
                D_80149333)
