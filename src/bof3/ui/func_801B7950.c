#include "bof3/ui/game00_internal.h"

/* @source 0x801B7950
 * @behavior runs the shared work-area select service selectWorkSequenceForRoute7, then
 * advances the work-area handler index at offset 0x04 by one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801B7950(void)
{
    selectWorkSequenceForRoute7();
    g_game_work->field_04++;
}
