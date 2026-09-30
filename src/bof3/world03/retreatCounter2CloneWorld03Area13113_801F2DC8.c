#include "bof3/world/area00813_internal.h"
#include "game/counter_step.h"

extern volatile u16 counter2;

/* @source 0x801F2DC8
 * @behavior marks the local counter active and retreats it by 20.
 * @status exact
 * @match 100.00
 * @residual none
 */
COUNTER_RETREAT(retreatCounter2CloneWorld03Area13113_801F2DC8, counter2,
                D_80149333)
