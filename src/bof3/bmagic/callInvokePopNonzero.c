#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F0BCC[])(void); /* @source 0x801F0BCC @kind data */

extern void renderComposeWorkMatrix(void); /* @source 0x801EF440 @kind function */
extern void renderFadeGouraudQuads(void); /* @source 0x801F02CC @kind function */

/* @source 0x801F0140
 * @behavior Calls the handler indexed by unsigned work byte 2 through the
 * table at 0x801F0BCC, without a bounds check. Reloads the work pointer from
 * scratchpad cell 0x1F800044 after the handler returns. When work byte 0 is
 * nonzero, invokes 0x801EF440, 0x801F02CC, and PopMatrix in that order.
 * Takes no arguments and returns nothing. The payload word at 0x801F0B9C
 * names this entry.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callInvokePopNonzero(void) {
    u8 *step_record;
    u8 *active_record;

    step_record = g_battle_work;
    D_801F0BCC[step_record[2]]();
    active_record = g_battle_work;
    if (active_record[0] != 0) {
        renderComposeWorkMatrix();
        renderFadeGouraudQuads();
        PopMatrix();
    }
}
