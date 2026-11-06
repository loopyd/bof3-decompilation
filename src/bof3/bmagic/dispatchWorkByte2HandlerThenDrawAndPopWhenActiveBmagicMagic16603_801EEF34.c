#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materializes the cell
 * address for each of the two reads in this function (lui $vN,0x1F80 / lw
 * $vN,68($vN)), so the cell is declared volatile, matching the exact bmagic
 * sibling decrementMotionCountdownAndSubtractMotionPairThenAdvanceWorkByte2
 * BmagicMagic00800_801F04DC. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Overlay handler table for work-record byte 2. The code-pointer run starts one
 * word earlier at 0x801EF718, which the plain step dispatcher of this target
 * indexes; this function indexes the second entry of the run. */
extern void (*D_801EF71C[])(void); /* @source 0x801EF71C @kind data */

/* Projects the work record's position and installs the resulting transform. */
extern void func_801EF284(void);

/* Draws the ten-panel textured band of the active work record. */
extern void func_801EF380(void);

/* @source 0x801EEF34
 * @behavior Reads the work record published at the scratchpad pointer cell
 *           0x1F800044 and uses its unsigned byte at offset 0x2, scaled by
 *           four, to select one entry of the handler table based at
 *           0x801EF71C, which it calls with no arguments. When the handler
 *           returns it reads the cell again in a second load; if the record
 *           byte at offset 0x0 and the byte at offset 0x2 are both non-zero it
 *           calls func_801EF284, then func_801EF380, then PopMatrix. Reads no
 *           other state, stores nothing and returns nothing. The 0x18-byte
 *           frame only keeps $ra across the indirect call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawAndPopWhenActiveBmagicMagic16603_801EEF34(void) {
    u8 *step_record;
    u8 *draw_record;

    step_record = g_battle_work;
    D_801EF71C[step_record[2]]();
    draw_record = g_battle_work;
    if (draw_record[0] != 0 && draw_record[2] != 0) {
        func_801EF284();
        func_801EF380();
        PopMatrix();
    }
}
