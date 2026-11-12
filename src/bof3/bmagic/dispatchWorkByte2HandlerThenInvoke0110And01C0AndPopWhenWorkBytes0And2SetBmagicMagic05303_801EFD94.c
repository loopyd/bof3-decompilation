#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell at 0x1F800044, bound by this target's map
 * through the shared symbols support file and read by every step handler in the
 * overlay. The original re-materialises the cell with its own lui/lw pair before
 * the frame setup and again after the dispatched handler returns, so the two
 * reads stay separate instead of folding one address into a saved register. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Handler table for work-record byte 2. The code-pointer run starts at
 * 0x801F0550 (0x801EF0A8, 0x801EF174, 0x801EF420, 0x801EF5C0, 0x801EF674,
 * 0x801EF6D8, 0x801EFD94, 0x801EFE1C, 0x801EFF24, 0x801F0018), which the
 * byte-1 dispatchers at 0x801EF064 and 0x801EFD50 index from 0x801F0550 and
 * 0x801F0568; this function indexes the same run from 0x801F056C, so the extern
 * is declared at the indexed entry. The generated undefined_syms_auto.txt row
 * names this word. */
extern void (*D_801F056C[])(void); /* @source 0x801F056C @kind data */

/* @source 0x801F0110 */
extern void func_801F0110(void);
/* @source 0x801F01C0 */
extern void func_801F01C0(void);

/* @source 0x801EFD94
 * @behavior Dispatches one step of the overlay work handler from the work
 *   record published at the scratchpad pointer cell 0x1F800044. The record is
 *   read once before the 0x18-byte frame is created; its unsigned byte at
 *   offset 0x2, scaled by four, selects one entry of the handler table based at
 *   0x801F056C and that entry is called with no arguments, and the original
 *   does not bound the index. The pointer cell is then read a second time, and
 *   only when the record's byte at offset 0x0 and its byte at offset 0x2 are
 *   both non-zero does the active arm run: it calls 0x801F0110, then
 *   0x801F01C0, and finally pops the GTE matrix stack with PopMatrix. The body
 *   makes no object stores, takes no arguments and returns nothing. The word at
 *   0x801F0568 of the same pointer run names this callback, which is the first
 *   entry the byte-1 dispatcher at 0x801EFD50 indexes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvoke0110And01C0AndPopWhenWorkBytes0And2SetBmagicMagic05303_801EFD94(void) {
  u8 *step_record;
  u8 *draw_record;

  step_record = g_battle_work;
  D_801F056C[step_record[2]]();
  draw_record = g_battle_work;
  if (draw_record[0] != 0 && draw_record[2] != 0) {
    func_801F0110();
    func_801F01C0();
    PopMatrix();
  }
}
