#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind the
 * same word; it lives outside this overlay, so it keeps its address-qualified
 * name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EF084
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 * and, only when the work object's byte at offset 0xB is zero, raises bit 0x4
 * of the shared battle global halfword at 0x801462E8 with a halfword
 * read-modify-write and then invokes the shared helper at 0x801E5988 with no
 * arguments, the store filling the call's delay slot. Its address is the third
 * word (index 2) of the three-entry code pointer run at 0x801EEC04, whose words
 * are 0x801EEC70, 0x801EEE14 and 0x801EF084 and which 0x801EEC10 copies into
 * its own frame and dispatches on the work object's byte 1; no other word in
 * this payload holds 0x801EF084 and neither this payload nor the main image has
 * a jal to it. The pointer cell is loaded before the 0x18-byte frame is
 * created, that frame only keeps $ra across the helper call, and the halfword
 * address stays in one register across the read-modify-write, so it is written
 * through a local pointer. The whole body is byte-identical to the exact
 * siblings at 0x801F01E4 in emi/bmagic/magic008/00, 0x801EF284 in
 * emi/bmagic/magic067/03, 0x801EEF38 in emi/bmagic/magic097/03, 0x801EF61C in
 * emi/bmagic/magic131/03 and 0x801EED24 in emi/bmagic/magic143/03. Nothing is
 * returned and no other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic03803_801EF084(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] == 0) {
    u16 *flags = (u16 *)&D_801462E8;

    *flags |= 4;
    func_801E5988();
  }
}
