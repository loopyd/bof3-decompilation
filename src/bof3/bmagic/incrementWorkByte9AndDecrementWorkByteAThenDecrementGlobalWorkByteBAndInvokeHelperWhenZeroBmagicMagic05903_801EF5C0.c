#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, bound by this target's map.
 * The cell is re-evaluated for each access group because the byte store that
 * precedes the second group goes through the pointer the cell supplied, so the
 * original re-materializes `lui $v1,0x1F80` / `lw $v1,0x44($v1)` exactly twice.
 * Declaration style follows the exact same-target sibling
 * incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndClear
 * WorkBytesWhenZeroBmagicMagic05903_801F012C (0x801F012C), whose 120-byte body
 * is identical to this span at every word except the call word at +0x60. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the overlay's remaining assembly loads it, and the name is its own
 * D_<address> form, so the binding needs no map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared battle helper at a fixed address outside this overlay's image. The
 * overlay's remaining assembly still calls it, so the generated undefined-funcs
 * map keeps the binding and no map row is owed. */
void func_801E5988(void);

/* @source 0x801EF5C0
 * @behavior Per-frame handler of the battle work object published at the
 *           scratchpad pointer cell 0x1F800044. It increments that object's
 *           unsigned byte 0x09 by one at byte width (lbu / addiu 1 / sb),
 *           re-reads the cell and subtracts one from unsigned byte 0x0A the
 *           same way, then tests the stored byte truncated to eight bits
 *           (andi $v0,$v0,0xFF / bnez), so the truncated register - not a
 *           reload - supplies the compared value. Only when byte 0x0A reaches
 *           zero does it read the shared battle work record pointer at
 *           0x801EB4E0, decrement that record's byte 0x0B by one at byte width
 *           and invoke the shared helper at 0x801E5988, the store filling the
 *           call delay slot. The first cell load precedes frame creation
 *           because the increment needs no stack; the 0x18-byte frame only
 *           keeps $ra across the call. Takes no arguments, returns nothing and
 *           reads or writes no other state. Bytes 0x801EF5C0..0x801EF637 are
 *           identical to the exact sibling
 *           incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteB
 *           AndInvokeHelperWhenZeroBmagicMagic14503_801EF0A0 at every word,
 *           proven with one payload comparison over the 0x78-byte span. The
 *           clone adapter declined that representative because the generated
 *           body uses the SPAD_PTR_TABLE macro, so this source was seeded from
 *           the same-target exact sibling at 0x801F012C, which shares the cell
 *           reload shape and differs only in the callee it names.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic05903_801EF5C0(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  work[0x09] += 1;
  work = g_battle_work;
  counter = work[0x0A] - 1;
  work[0x0A] = counter;
  if (counter == 0) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
