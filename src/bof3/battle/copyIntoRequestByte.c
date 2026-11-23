#include "bof3/battle/battle15_internal.h"

/* Resource request helper of the main executable, outside this overlay's image:
 * this target's generated undefined_funcs_auto row binds 0x8014D6B8, which its
 * remaining asm spans still call. The parameter is byte-wide, the arity the
 * exact bmagic lift invokeBattlerStatusHelperThenSetGlobalWorkByteBToFFAndWork
 * Byte2To6... (0x801F1474) declares for the same helper; a byte value plus a
 * constant argument truncates to 0xFF there and here, which is the andi this
 * body emits after its addiu. */
extern void func_8014D6B8(u8 request); /* @source 0x8014D6B8 */

/* @source 0x800A85A8
 * @behavior Handler 0 of the shared work-byte-1 handler table at 0x800B50B0,
 *   the run the dispatcher at 0x800A8540 indexes with work byte 1 after scaling
 *   it by four; no jal in this payload names this span, its only in-image
 *   reference is that table word. It loads the scratchpad work object published
 *   at the pointer cell 0x1F800044, stores that object's word at 0x3C plus
 *   0x4000000 into its own word at 0x20 (lw / lui 0x400 / addu / sw), requests
 *   resource id byte 0x08 of that object plus 0x38 through the main-executable
 *   helper func_8014D6B8, then re-reads the cell to set object byte 0x0B to
 *   0x10 and again to advance object byte 0x01 by one. Takes no arguments,
 *   returns nothing and reads or writes no other state. The roles of the words
 *   at 0x20 and 0x3C, of byte 0x08 and of byte 0x0B are not established here,
 *   so the name describes the stores.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyIntoRequestByte(
    void) {
  u8 *work;
  u8 *step;
  u8 *tail;
  u32 value;

  work = g_battle_work;
  value = *(u32 *)(work + 0x3C) + 0x4000000;
  *(u32 *)(work + 0x20) = value;
  func_8014D6B8(work[8] + 0x38);
  step = g_battle_work;
  step[0xB] = 0x10;
  tail = g_battle_work;
  tail[1]++;
}
