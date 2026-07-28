#include "internal.h"

/* @source 0x801F3280
 * @behavior resets the local selection counter, marks it active, and stops effect 0.
 */
void func_801F3280(void)
{
    WORLD00_AREA026_13_D_8014932A = 0;
    WORLD00_AREA026_13_D_80149333 = 2;
    game_stop_selection_fx(0u, 0);
}
