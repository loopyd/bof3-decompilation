#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The original reads the cell once per access, so this function's
 * single load stays one plain pointer load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind
 * the same word (BATTLE_GLOBAL_HALF_62E8); it lives outside this overlay, so it
 * keeps its address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EF61C
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 * and, only when the work object's byte at offset 0xB is zero, raises bit 0x4
 * of the shared battle global halfword at 0x801462E8 and then invokes the
 * shared helper at 0x801E5988 with no arguments. Its address is the eighth
 * word (index 7) of the eight-entry code pointer run at 0x801EEC04 in this
 * target's leading table segment, whose words are 0x801EEDEC, 0x801EEF64,
 * 0x801EF000, 0x801EF110, 0x801EF1C0, 0x801EF3D0, 0x801EF488 and
 * 0x801EF61C; no dispatched arm or jal inside this payload reaches it. The
 * pointer cell is loaded before the 0x18-byte frame is created, that frame
 * only keeps $ra across the helper call, and the halfword address stays in one
 * register across the read-modify-write, so it is written through a local
 * pointer. Nothing is returned and no other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic13103_801EF61C(void) {
  u8 *work = g_battle_work;

  if (work[0xB] == 0) {
    u16 *flags = (u16 *)&D_801462E8;

    *flags |= 4;
    func_801E5988();
  }
}
