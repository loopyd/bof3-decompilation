#include "bof3/bof3.h"

/* Shared status-flag helper in the main executable, outside this overlay's
 * image. The payload only calls it, so the prototype keeps the raw address name
 * from this target's generated undefined_funcs_auto.txt and needs no map row.
 * Its first argument is the battle target byte at 0x80146384, the second the
 * flag word to raise, exactly the call form the exact siblings
 * raiseBattleTargetFlag10WhenWorkByte9SetThenAdvanceWorkByte1BmagicMagic10003_801EED9C
 * and
 * raiseBattleTargetFlag20ClearBytes9AThenAdvanceWorkByte1BmagicMagic09403_801EEF1C
 * record for the same helper. */
extern void func_800A9610(u8 arg0, s32 arg1); /* @source 0x800A9610 @kind function */

/* Battle target byte the shared flag helper is handed, in main RAM outside this
 * overlay's image where the other overlays' exact lifts read the same byte. The
 * address is bound by this target's generated undefined_syms_auto.txt, which the
 * remaining assembly span func_801EEF3C still references, so this lift needs no
 * target-map row. */
extern u8 D_80146384; /* @source 0x80146384 @kind data */

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * address (lui $vN,0x1F80 / lw $vN,68($vN)) before each read, and the stores
 * after a read go through the pointer that read supplied, so each access group
 * spells the named cell again instead of reusing a saved copy. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad halfword at offset 0. The original materialises the address at the
 * store site with a single `lui $at,0x1F80` and writes through it, and no row of
 * this target's map names the cell, so the D_-form declaration binds the fixed
 * address with no map row. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* @source 0x801EF2DC
 * @behavior Word 5 of the in-image code pointer run based at 0x801F0744
 *           (0x801EF1F4, 0x801EF1F4, 0x801EFA50, 0x801EFF28, 0x801EF27C,
 *           0x801EF2DC, 0x801EF37C, 0x801EFAD8), the handler table the
 *           work-byte-2 dispatcher indexes, and this payload names the span from
 *           outside the overlay: no jal and no other payload word names it. It
 *           reads the scratchpad work-object pointer cell at 0x1F800044 once,
 *           before its frame exists, and only when that object's unsigned byte 9
 *           reads 4 does it forward the battle target byte at 0x80146384 with
 *           the flag word 0x10 to the shared helper at 0x800A9610, the flag word
 *           filling the call delay slot. It re-reads the cell, stores that
 *           object's unsigned byte 9 scaled by 0x1C at halfword width into the
 *           scratchpad halfword at 0x1F800000, the address materialised at the
 *           store site with `lui $at,0x1F80`, then re-reads byte 9 through the
 *           same pointer, increments it at byte width and masks the stored
 *           register with andi 0xFF before comparing it against 0xC. Only when
 *           that counter has just reached 0xC does it re-read the cell and
 *           increment the work-object handler byte at offset 2 at byte width,
 *           which advances the work-byte-2 dispatcher to the next step. Takes no
 *           arguments, returns nothing, and its 0x18-byte frame keeps only $ra
 *           at 0x10 across the conditional call. It reads or writes no other
 *           state, and the reader of the published scratchpad halfword is
 *           outside this span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkByte9TimesTwentyEightToScratchpadHalfwordRaiseFlag10WhenByte9Is4ThenAdvanceWorkByte2WhenByte9Is12BmagicMagic05603_801EF2DC(void) {
  u8 *work;

  if (g_battle_work[0x09] == 4) {
    func_800A9610(D_80146384, 0x10);
  }
  work = g_battle_work;
  D_1F800000 = work[0x09] * 0x1C;
  if (++work[0x09] == 0xC) {
    g_battle_work[0x02] += 1;
  }
}
