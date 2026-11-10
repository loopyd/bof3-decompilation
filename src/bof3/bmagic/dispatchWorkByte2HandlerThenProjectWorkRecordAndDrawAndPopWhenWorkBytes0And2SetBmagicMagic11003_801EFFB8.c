#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* The last two words of the handler run at 0x801F046C. The original
 * indexes from this interior word without checking the byte-2 index. */
extern void (*D_801F049C[])(void); /* @source 0x801F049C @kind data */

/* Pushes the matrix stack, projects the work position and installs the
 * resulting transform. Its own body reads the record, not arguments. */
extern void func_801F0138(void); /* @source 0x801F0138 @kind function */

/* Emits translucent textured geometry using the work record. */
extern void func_801F01E8(void); /* @source 0x801F01E8 @kind function */

/* @source 0x801EFFB8
 * @behavior Entry 2 of the byte-1 handler run at 0x801F046C. Reads the
 *           scratchpad work pointer at 0x1F800044 and dispatches through
 *           the table at 0x801F049C using unsigned work byte 2, without
 *           bounding the index. Re-reads the pointer after the handler and
 *           tests work bytes 0 and 2 in that order. When both are nonzero,
 *           calls the transform installer at 0x801F0138, the draw pass at
 *           0x801F01E8 and PopMatrix. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenProjectWorkRecordAndDrawAndPopWhenWorkBytes0And2SetBmagicMagic11003_801EFFB8(void) {
    D_801F049C[g_battle_work[2]]();
    if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
        func_801F0138();
        func_801F01E8();
        PopMatrix();
    }
}
