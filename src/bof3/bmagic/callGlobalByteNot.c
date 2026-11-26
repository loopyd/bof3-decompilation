#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell, shared by this overlay's step handlers.
 * The row lives in this target's map. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address, so
 * the name needs no map row, and every access here is byte-addressed, so the
 * local declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Code pointer table the work-byte-2 step handlers are dispatched from. Its base
 * is word 2 of the 15-entry handler run starting at 0x801F0CA8, which no map row
 * owns, so the address is bound by the target's generated undefined-symbol table,
 * exactly as the exact sibling setDrawModeThenDispatchWorkByte2Handler...801EFD20
 * binds word 6 of that same run. */
extern void (*D_801F0CB0[])(void); /* @source 0x801F0CB0 @kind data */

/* Target-local block at 0x801EF44C; its own row is still assembly, so the name
 * stays address-anchored. */
extern void stateSelectDrawMode(void); /* @source 0x801EF44C @kind function */

/* @source 0x801EF054
 * @behavior Dispatches one work-byte-2 step of the overlay sequence: reads the
 *           work-record pointer cell at 0x1F800044, scales the record's byte 2 by
 *           four and calls that entry of the local code pointer table based at
 *           0x801F0CB0 with no arguments. After the handler returns it reads the
 *           shared battle work record pointer at 0x801EB4E0 once and tests that
 *           record's byte 0xB; only when it is not 0xFF does it read the
 *           scratchpad cell a second time and test the work record's byte 2,
 *           which the handler may have advanced. When that byte is nonzero it
 *           calls the target-local block at 0x801EF44C. Takes no arguments and
 *           returns nothing; its 0x18-byte frame exists only to keep the return
 *           address across the two calls. The first word of the handler run based
 *           at 0x801F0CA8, 0x801EF054, names this span. Statement order, operand
 *           forms, offsets and declaration style follow the exact sibling
 *           dispatchWorkByte2HandlerThenInvokeHelpersWhenWorkBytes0And2Set...801EFDC8,
 *           whose guarded tail has the same shape; the table word, callee and
 *           compared literal are this span's own. A byte scan of the shipped
 *           payloads found no byte-identical twin of this span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callGlobalByteNot(
    void) {
  D_801F0CB0[g_battle_work[2]]();
  if (D_801EB4E0[0xB] != 0xFF && g_battle_work[2] != 0) {
    stateSelectDrawMode();
  }
}
