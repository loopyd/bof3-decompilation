#include "bof3/bof3.h"

void func_800A9820(u8 arg0);
void func_801E5988(void);

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * func_800A9820 consumes. It lives outside this overlay, so it keeps its
 * address-qualified name and binds through the auto-generated undefined
 * symbol list of this target. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword at 0x801462E8. It lives outside this overlay,
 * so it keeps its address-qualified name; the original reads it unsigned, ors
 * 0x4 and stores it back through a re-materialised base register ($at), so the
 * raise is a plain global read-modify-write with no pointer local. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays, bound at 0x1F800044 by this target's map. The original reads the
 * cell once, before the frame is created, so it stays one plain pointer load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EED90
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once,
 *           before the 0x18-byte frame that keeps only $ra at 0x10 is created,
 *           and returns immediately through the jr $ra at 0x801EEDE8 when the
 *           work object's unsigned byte at offset 0xB is non-zero. Otherwise it
 *           loads the shared battle byte at 0x80146384 unsigned and passes it
 *           to the shared helper func_800A9820, raises bit 0x4 of the shared
 *           battle global halfword at 0x801462E8 with an unsigned halfword
 *           load, ori and halfword store, and then invokes the shared helper
 *           func_801E5988. It takes no arguments, returns nothing and writes no
 *           other state. Every literal the 0x60-byte span encodes (0x1F800044,
 *           work offset 0xB, 0x80146384, 0x801462E8, bit 0x4 and both call
 *           targets) is the same here, and the span is instruction-identical to
 *           the exact lifts at
 *           emi/bmagic/magic159/03@0x801EED84,
 *           emi/bmagic/magic106/03@0x801EEE80,
 *           emi/bmagic/magic072/03@0x801EF2DC and
 *           emi/bmagic/magic218/03@0x801EEDB8, so statement order, operand
 *           forms, declaration style and the name all come from those sources.
 *           No word in this payload holds 0x801EED90 and no jal in it targets
 *           the span, so it has no direct caller here; the role of work-object
 *           byte 0xB beyond its zero test is not established. The boundary
 *           comes from this overlay's own Splat segment, whose next entry starts
 *           at 0x801EEDF0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic17303_801EED90(void) {
  u8 *work = g_battle_work;

  if (work[0xB] == 0) {
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
