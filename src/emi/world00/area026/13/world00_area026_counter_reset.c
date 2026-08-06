#include "internal.h"

/* @source 0x801F3280
 * @behavior resets the local selection counter, marks it active, and stops effect 0.
 */
void world00_area026_counter_reset(void)
{
    world00_area026_counter = 0;
    D_80149333 = 2;
    game_stop_selection_fx(0u, 0);
}
