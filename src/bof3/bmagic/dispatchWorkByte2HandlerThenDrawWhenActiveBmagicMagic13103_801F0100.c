#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The cell is read once per access, so the original re-materializes
 * its address (lui $vN,0x1F80 / lw $vN,68($vN)) for each of the two accesses. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Five-entry overlay step-handler table for work-object byte 2. */
extern void (*dispatchHandlerTable801F1BF4[])(void); /* @source 0x801F1BF4 @kind data */

/* Entry drawing routine of this overlay. */
extern void func_801F030C(void);

/* @source 0x801F0100
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 and
 * uses the work-object byte at offset 0x2, scaled by four, to select one entry
 * of the five-entry handler table at 0x801F1BF4, which it calls with no
 * arguments. When the handler returns it reads the pointer cell again in a
 * second load; if the work-object byte at offset 0x0 and the byte at offset 0x2
 * are both non-zero it calls the overlay's drawing routine func_801F030C.
 * Reads no other state, stores nothing and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawWhenActiveBmagicMagic13103_801F0100(void) {
    u8 *step_work;
    u8 *gate_work;

    step_work = g_battle_work;
    dispatchHandlerTable801F1BF4[step_work[2]]();
    gate_work = g_battle_work;
    if (gate_work[0] != 0 && gate_work[2] != 0) {
        func_801F030C();
    }
}
