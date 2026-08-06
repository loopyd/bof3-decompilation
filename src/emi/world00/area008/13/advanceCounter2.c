#include "internal.h"
#include "game/counter_step.h"

/* @source 0x801F4578
 * @behavior marks the local counter active and advances it by 20.
 */
COUNTER_ADVANCE(advanceCounter2, counter2,
                D_80149333)
