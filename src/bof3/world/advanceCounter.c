#include "bof3/world/area02613_internal.h"
#include "game/counter_step.h"

/* @source 0x801F3258
 * @behavior marks the local counter active and advances it by 20.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
COUNTER_ADVANCE(advanceCounter, counter,
                D_80149333)
