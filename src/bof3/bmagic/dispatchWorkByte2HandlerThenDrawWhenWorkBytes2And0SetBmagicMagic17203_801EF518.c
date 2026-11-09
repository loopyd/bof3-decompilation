#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell at 0x1F800044. The original
 * re-materialises the cell with its own lui/lw pair at both uses, so the cell is
 * read through this named row rather than through the constant-address
 * accessor, as in the exact bmagic siblings
 * dispatchWorkByte2HandlerThenDrawWorkRecordAndPopWhenWorkBytes2And0Set
 * BmagicMagic09703_801EEFC8 and
 * decrementWorkByte9ByTwoThenDecrementGlobalWorkByteBAndInvokeHelperWhenZero
 * BmagicMagic17203_801EF4BC, which read the same cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Word 7 of the fourteen-word code-pointer run this overlay stores at
 * 0x801F0B74 (the run the byte-1 dispatcher at 0x801EEE38 indexes from its
 * head). The work record's byte 2 scales by four and is added to 0x801F0B90,
 * so the extern is declared at the indexed word and the run stays the single
 * D_801F0B74 row in this target's map; the generated undefined_syms_auto.txt
 * row for 0x801F0B90 already names this word. */
extern void (*D_801F0B90[])(void); /* @source 0x801F0B90 @kind data */

/* Shared main-executable helper at 0x800AF764. This overlay calls it once per
 * active step with no arguments and no result; the generated
 * undefined_funcs_auto.txt row for that address binds it. No lifted body for it
 * existed in this tree when this seed was written, so it keeps its raw name. */
void func_800AF764(void);

/* This overlay's own record draw step, still an asm row at 0x801F0908 in this
 * target: it selects a draw mode from GetGraphType, installs it on the prim
 * cursor and queues the record's primitives. Kept at its raw name because the
 * span has no authored body. */
void func_801F0908(void);

/* @source 0x801EF518
 * @behavior Dispatches one step of the overlay work handler and then draws the
 *   active record. The work record is read from the scratchpad pointer cell at
 *   0x1F800044, its unsigned byte at offset 0x2 is scaled by four and selects
 *   one entry of the handler table word at 0x801F0B90, which is called with no
 *   arguments; the original does not bound the index. The cell is then re-read
 *   and the record's bytes at offsets 0x0 and 0x2 are loaded and ANDed
 *   together, so only when both are non-zero does the arm run: it calls the
 *   shared main-executable helper at 0x800AF764 and then this overlay's record
 *   draw step at 0x801F0908, both with no arguments. The cell is re-read rather
 *   than kept live because the dispatched handler may republish it, which is why
 *   the original materialises it twice. The 0x18-byte frame only keeps $ra at
 *   0x10 across the calls, and the pointer-cell load is issued before frame
 *   creation. The 120-byte span takes nothing, returns nothing and reads or
 *   writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawWhenWorkBytes2And0SetBmagicMagic17203_801EF518(void) {
  D_801F0B90[g_battle_work[2]]();
  if ((g_battle_work[0] & g_battle_work[2]) != 0) {
    func_800AF764();
    func_801F0908();
  }
}
