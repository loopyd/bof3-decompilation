#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell at 0x1F800044. The original loads it
 * once, before the 0x18-byte frame is created, into $v0 and reads work byte
 * 0xB through that same base, so a single named cell read carries the guard.
 * Declaration style follows the exact same-overlay sibling
 * incrementWorkByte1WhenWorkByteBBelow2BmagicMagic01303_801EEE68. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle global halfword at 0x801462E8, raised through bit 0x4. It lies
 * outside this overlay, so it keeps the address-qualified name this target's
 * generated undefined_syms_auto.txt already binds. The original materializes
 * the address once (lui/addiu) and keeps it in $v1 across the read-modify-write,
 * which the local pointer below reproduces. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEF84
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           and, only when the work object's byte at offset 0xB is zero, raises
 *           bit 0x4 of the shared battle global halfword at 0x801462E8 and then
 *           invokes the shared helper at 0x801E5988 with no arguments. The
 *           pointer cell is loaded before the 0x18-byte frame is created, that
 *           frame only keeps $ra across the helper call, and the halfword store
 *           lands in the call's delay slot, so the raise precedes the call.
 *           Nothing is returned and no other state is read or written. Its 76
 *           bytes 0x801EEF84..0x801EEFD0 compare equal to the original bytes of
 *           the exact lift raiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic00800_801F01E4
 *           of bmagic/magic008/00, which supplied the statement order, operand
 *           forms and this behavior name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic01303_801EEF84(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] == 0) {
    u16 *flags = (u16 *)&D_801462E8;

    *flags |= 4;
    func_801E5988();
  }
}
