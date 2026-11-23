#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind the
 * same word; it lives outside this overlay, so it keeps its address-qualified
 * name, which the target's generated undefined-syms map binds through the row
 * D_801462E8 = 0x801462E8. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EFA84
 * @behavior Overlay work handler, word index 16 of this overlay's leading
 *           24-word code pointer run at 0x801EEC04 and the third word (index 2)
 *           of the three-word table at 0x801EEC3C that the dispatcher
 *           dispatchWorkByte1Fade copies into its
 *           frame and indexes by the work object's byte at offset 1. Raises bit
 *           0x4 of the shared battle global halfword at 0x801462E8 with an
 *           unsigned halfword read-modify-write and then invokes the shared
 *           helper at 0x801E5988 with no arguments, unconditionally. The
 *           original materializes the halfword address once in a register
 *           because both the load and the store use it, so the access is
 *           written through a local pointer; the halfword store fills the call
 *           delay slot, and the 0x18-byte frame only keeps $ra at 0x10. Takes no
 *           arguments, returns nothing, and reads or writes no other state. Its
 *           0x34-byte body is instruction-for-instruction identical to the
 *           sibling handler at 0x801EEFA4 of this overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4InvokeHelper(void) {
  u16 *flags = (u16 *)&D_801462E8;

  *flags |= 4;
  func_801E5988();
}
