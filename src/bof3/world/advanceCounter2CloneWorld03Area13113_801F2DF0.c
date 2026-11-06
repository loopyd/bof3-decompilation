#include "bof3/world/area13113_internal.h"
#include "game/counter_step.h"

/* @source 0x801F2DF0
 * @behavior Overlay counter step: marks the shared counter active by storing 2
 * into the area status byte at 0x80149333, then advances counter2 by 0x14.
 * @status exact
 * @match 100.00
 * @residual none
 */
COUNTER_ADVANCE(advanceCounter2CloneWorld03Area13113_801F2DF0, counter2,
                D_80149333)
