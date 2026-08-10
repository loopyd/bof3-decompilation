#include "bof3/world/area02613_internal.h"

/* @source 0x801F3280
 * @behavior resets the local selection counter, marks it active, and stops effect 0.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void resetCounter(void)
{
    selectionCounter = 0;
    D_80149333 = 2;
    game_stop_selection_fx(0u, 0);
}
