#include "bof3/bof3.h"

void func_800A9820(u8 arg0);
void func_801E5988(void);

/* Scratchpad work-object pointer cell at 0x1F800044. This target's map already
 * binds it as g_battle_work, the name its other lifts use, so the symbolic
 * address folds into the single `lui $v0,0x1F80` / `lw $v0,0x44($v0)` pair the
 * body issues before it creates its frame. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * func_800A9820 consumes. It lives outside this overlay, so it keeps its
 * address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind the
 * same word (BATTLE_GLOBAL_HALF_62E8); it lives outside this overlay, so it
 * keeps its address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEE30
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once,
 *           before the frame exists, and returns immediately through the shared
 *           epilogue when the work object's byte at offset 0xB is non-zero.
 *           Otherwise it loads the shared battle byte at 0x80146384 unsigned
 *           and passes it to the shared helper func_800A9820, raises bit 0x4 of
 *           the shared battle global halfword at 0x801462E8 (unsigned halfword
 *           load, ori, halfword store through a re-materialised $at base) and
 *           then invokes the shared helper func_801E5988. It takes no
 *           arguments, returns nothing and writes no other state; the
 *           0x18-byte frame only keeps $ra at 0x10 across the two calls. The
 *           96 bytes are byte-identical to the already-exact sibling at
 *           emi/bmagic/magic106/03@0x801EEE80, so this source was seeded from
 *           that proven body for this overlay's own offset and identity. The
 *           role of work byte 0xB stays offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic06903_801EEE30(
    void) {
  u8 *work;

  work = g_battle_work;
  if (work[0xB] == 0) {
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
