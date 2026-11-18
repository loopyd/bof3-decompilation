#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * address (lui $vN,0x1F80 / lw $vN,68($vN)) before each of its two reads, and
 * both stores after a read go through the pointer that read supplied, so each
 * access group spells the named cell again instead of reusing a saved copy. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad halfword at offset 0. The original materialises the address at
 * the store site with a single `lui $at,0x1F80` and writes through it, and no
 * row of this target's map names the cell, so the D_-form declaration binds
 * the fixed address with no map row. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address
 * because other remaining assembly spans load it, so the name needs no map row,
 * and every access here is byte-addressed, so the local declaration keeps the
 * byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF37C
 * @behavior Word 6 of the in-image code pointer run based at 0x801F0744
 *           (0x801EF1F4, 0x801EF1F4, 0x801EFA50, 0x801EFF28, 0x801EF27C,
 *           0x801EF2DC, 0x801EF37C, 0x801EFAD8, 0x801EFB38, 0x801EFBA4,
 *           0x801EFBF4), which this payload reaches from outside the overlay:
 *           no jal and no other payload word names this span. It reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once, before the
 *           frame exists, and stores that object's unsigned byte 9 widened as
 *           (byte << 2) + 0x120 at halfword width into the scratchpad halfword
 *           at 0x1F800000, the address materialised at the store site with
 *           `lui $at,0x1F80`. It then increments that same object's byte 0xA at
 *           byte width through the pointer the first read supplied. It re-reads
 *           the cell, increments byte 9 at byte width and masks the stored
 *           register with andi 0xFF before comparing it against 0x28; only when
 *           that counter has just reached 0x28 does it read the shared battle
 *           work record pointer at 0x801EB4E0, decrement that record's byte 0xB
 *           by one at byte width and invoke the shared helper at 0x801E5988,
 *           the store filling the call delay slot. Takes no arguments, returns
 *           nothing, and its 0x18-byte frame keeps only $ra at 0x10 across the
 *           call. It reads or writes no other state. Argument roles stay
 *           structural, and the reader of the published scratchpad halfword is
 *           outside this span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkByte9TimesFourPlus120ToScratchpadHalfwordIncrementWorkByteAAndWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenByte9Is28BmagicMagic05603_801EF37C(void) {
  u8 *work;

  work = g_battle_work;
  D_1F800000 = (work[0x09] << 2) + 0x120;
  work[0x0A] += 1;
  if (++g_battle_work[0x09] == 0x28) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
