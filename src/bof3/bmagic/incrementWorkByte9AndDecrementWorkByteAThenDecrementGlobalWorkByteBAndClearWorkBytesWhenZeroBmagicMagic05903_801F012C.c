#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, bound by this target's map.
 * The cell is re-evaluated for each access group because the byte store that
 * precedes the second group goes through the pointer the cell supplied, so the
 * original re-materializes `lui $v1,0x1F80` / `lw $v1,0x44($v1)` exactly twice.
 * Declaration style follows the exact same-target sibling
 * incrementWorkByte9AndAdvanceWorkByte2AtA0BmagicMagic05903_801F00DC. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the overlay's remaining assembly loads it, and the name is its own
 * D_<address> form, so the binding needs no map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

void clearWorkBytesBmagicMagic05903_801F04F4(void);

/* @source 0x801F012C
 * @behavior Per-frame handler of the battle work object published at the
 *           scratchpad pointer cell 0x1F800044. It increments that object's
 *           unsigned byte 0x09 by one at byte width (lbu / addiu 1 / sb),
 *           re-reads the cell and subtracts one from unsigned byte 0x0A the
 *           same way, then tests the stored byte truncated to eight bits
 *           (andi $v0,$v0,0xFF / bnez), so the truncated register - not a
 *           reload - supplies the compared value. Only when byte 0x0A reaches
 *           zero does it read the shared battle work record pointer at
 *           0x801EB4E0, decrement that record's byte 0x0B by one at byte width
 *           and invoke this overlay's work-byte clear routine at 0x801F04F4,
 *           the store filling the call delay slot. The first cell load precedes
 *           frame creation because the increment needs no stack; the 0x18-byte
 *           frame only keeps $ra across the call. Takes no arguments, returns
 *           nothing and reads or writes no other state. No jal in this payload
 *           reaches it: its address is entry 4 of the handler table based at
 *           0x801F0680, the word at 0x801F0690 inside the textbin row T_801F065C
 *           that the byte-1 dispatcher
 *           dispatchStateByte1Table0680BmagicMagic05903_801EFE48 indexes.
 *           Bytes 0x801F012C..0x801F01A3 are identical to the exact sibling
 *           incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteB
 *           AndInvokeHelperWhenZeroBmagicMagic14503_801EF0A0 at every word
 *           except the call word at +0x60, which names a different overlay's
 *           helper; that one-word difference is why the clone lever offered no
 *           eligible source and this body was seeded from the sibling shape.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndClearWorkBytesWhenZeroBmagicMagic05903_801F012C(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  work[0x09] += 1;
  work = g_battle_work;
  counter = work[0x0A] - 1;
  work[0x0A] = counter;
  if (counter == 0) {
    D_801EB4E0[0x0B] -= 1;
    clearWorkBytesBmagicMagic05903_801F04F4();
  }
}
