#include "internal.h"
#include "game/counter_step.h"

/* @source 0x801F3230
 * @behavior marks the local counter active and retreats it by 20.
 */
COUNTER_RETREAT(world00_area026_counter_retreat, world00_area026_counter,
                D_80149333)
