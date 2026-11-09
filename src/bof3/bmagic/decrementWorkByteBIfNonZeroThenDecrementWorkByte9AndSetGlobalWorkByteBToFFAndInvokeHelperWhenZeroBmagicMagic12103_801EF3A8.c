#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * This body reads the cell twice and the original re-materialises the cell
 * address with its own lui / lw before each read, because the byte store in the
 * first arm can alias the pointer cell itself; the cell therefore stays a named
 * pointer rather than a cached local. This target's map binds the name. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * splat binds 0x801EB4E0 through this target's generated
 * undefined_syms_auto.txt, so no target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

void func_801E5988(void);

/* @source 0x801EF3A8
 * @behavior Per-frame step handler of the scratchpad work object published at
 *           the pointer cell 0x1F800044. It first decrements the object's byte
 *           0x0B only when that byte is non-zero (lbu / beqz, the byte-width
 *           addiu -1 in the branch delay slot, then sb), so the byte floors at
 *           zero instead of wrapping. It then reloads the pointer cell and
 *           decrements byte 0x09 unconditionally (lbu / addiu -1 / sb) and
 *           tests the stored byte truncated to eight bits (andi $v0,$v0,0xFF /
 *           bnez), so the truncated register - not a reload - supplies the
 *           compared value. Only when that countdown reaches zero does it read
 *           the shared battle work record pointer at 0x801EB4E0, write 0xFF into
 *           that record's byte 0x0B and invoke the shared helper at 0x801E5988,
 *           the 0xFF materialization filling the bnez delay slot and the store
 *           the call delay slot. The first cell load precedes frame creation
 *           because it needs no stack; the 0x18-byte frame only keeps $ra at
 *           0x10 across the call. Takes no arguments, returns nothing and reads
 *           or writes no other state. The 0x6C-byte body is the guarded
 *           counterpart of decrementWorkByte9ThenDecrementGlobalWorkByteB
 *           AndInvokeHelperWhenZeroBmagicMagic13103_801F02B0 (same byte 0x09
 *           countdown, same 0x801EB4E0 byte 0x0B arm, same helper call) and its
 *           tail is the 0xFF-store form of
 *           incrementWorkByteAThenSetGlobalWorkByteBToFFAt80AndInvokeHelper
 *           BmagicMagic10803_801EF130. Statement order, operand forms and
 *           declaration style follow those exact siblings; no byte-identical
 *           copy of this span exists in the repository's overlay payloads.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteBIfNonZeroThenDecrementWorkByte9AndSetGlobalWorkByteBToFFAndInvokeHelperWhenZeroBmagicMagic12103_801EF3A8(void) {
  if (g_battle_work[0x0B] != 0) {
    g_battle_work[0x0B]--;
  }
  if (--g_battle_work[0x09] == 0) {
    D_801EB4E0[0x0B] = 0xFF;
    func_801E5988();
  }
}
