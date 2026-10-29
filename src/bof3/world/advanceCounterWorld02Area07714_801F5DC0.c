#include "bof3/bof3.h"
#include "game/counter_step.h"

extern u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801F5DC0
 * @behavior Advances the shared 16-bit area counter at 0x8014932A by 0x14 and
 *           sets the shared state byte at 0x80149333 to 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
COUNTER_ADVANCE(advanceCounterWorld02Area07714_801F5DC0, D_8014932A, D_80149333)
