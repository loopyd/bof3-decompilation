#include "bof3/world/area13113_internal.h"
#include "game/counter_step.h"

/* @source 0x801F2DC8
 * @behavior marks the local counter active and retreats it by 20.
 * @status exact
 * @match 100.00
 * @residual none
 */
COUNTER_RETREAT(retreatCounter2Clone, counter2,
                D_80149333)
