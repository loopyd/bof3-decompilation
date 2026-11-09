#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, bound by this target's map row
 * g_battle_work. Declared as the named cell this target's lifted sources use: the
 * symbolic address folds into each load (lui $vN,0x1F80 / lw $vN,0x44($vN)), and
 * the original re-reads the cell before each access group because a store through
 * the pointer the cell supplied may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * splat binds 0x801EB4E0 through this target's generated
 * undefined_syms_auto.txt, so no target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF2BC
 * @behavior Entry 12 of this overlay's fourteen-entry work-byte-1 handler table
 *           based at 0x801F02A4, whose word 0x801F02D4 is the only in-image
 *           reference to this address, so the body has no jal caller. The
 *           dispatcher at 0x801EED98 indexes that table with the scratchpad
 *           work object's state byte 0x1 (lbu 1 / sll 2 / lw / jalr). The body
 *           reads the work-object pointer cell at 0x1F800044 once and, while the
 *           unsigned countdown byte 0x9 it loads through it is not zero,
 *           subtracts one at byte width (lbu 9 / addiu -1 in the delay slot of
 *           the beqz / sb 9). It then re-reads the cell, increments unsigned byte
 *           0xA at byte width and masks the value it just stored with andi 0xFF,
 *           so the truncated register - not a reload - supplies the compared
 *           value; only when that byte reaches 0xA0 does it read the shared
 *           battle work record pointer at 0x801EB4E0, subtract one from that
 *           record's byte 0xB at byte width, re-read the cell a third time and
 *           increment work byte 0x2 at byte width, advancing the dispatcher on
 *           work byte 2 to the next handler step. All three cell loads are
 *           required: each store goes through the pointer the cell supplied.
 *           Leaf with no frame, no call and no other state; takes no arguments
 *           and returns nothing. Statement order and declaration style follow
 *           the reviewed exact siblings on this target
 *           incrementWorkByteAThenIncrementWorkByte2WhenGlobalWorkByteBMatches
 *           WorkByte1BmagicMagic07903_801EFF58 (same table, same cell, same byte
 *           0xA increment, same 0x801EB4E0 record) and
 *           decrementWorkByte9Subtract40FromWorkDistanceThenResetByte9To20And
 *           AdvanceWorkByte2OnZeroBmagicMagic07903_801EF1DC (same table, same
 *           conditional byte 0x9 decrement, same work byte 0x2 advance), and the
 *           byte 0xA increment/compare spelling of
 *           incrementWorkByteAThenSetGlobalWorkByteBToFFAt80AndInvokeHelper
 *           BmagicMagic10803_801EF130. No byte-identical copy of this 140-byte
 *           span exists in the 10216 overlay functions under out/splat, and no
 *           reviewed source on another target carries this immediate set, so the
 *           clone lever offered no source.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenIncrementWorkByteAAndDecrementGlobalWorkByteBAndAdvanceWorkByte2AtA0BmagicMagic07903_801EF2BC(void)
{
  u8 tick;

  if (g_battle_work[9] != 0) {
    g_battle_work[9]--;
  }
  tick = (u8)(g_battle_work[0xA] + 1);
  g_battle_work[0xA] = tick;
  if (tick == 0xA0) {
    D_801EB4E0[0x0B] -= 1;
    g_battle_work[2]++;
  }
}
