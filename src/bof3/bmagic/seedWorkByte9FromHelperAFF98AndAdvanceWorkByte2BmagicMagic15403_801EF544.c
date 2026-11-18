#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, outside this overlay's
 * image and re-materialised by the original for both accesses below (lui
 * $v1,0x1F80 / lw $v1,68($v1) before the byte 0x9 store and again before the
 * byte 0x2 advance). This target's map binds no name there, so the cell is
 * reached through its address-anchored name, the same spelling the exact
 * siblings dispatchWorkByte2HandlerThenInvokeD290WhenWorkByte0Nonzero
 * BmagicMagic15403_801EF4B4 and
 * incrementWorkByte9AndAdvanceMotionPairThenAdvanceWorkByte2At4
 * BmagicMagic15403_801EF380 bind it with. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared main-executable helper at 0x800AFF98, also bound by this target's
 * generated function map. It takes no arguments and returns the battle-
 * participant lookup byte this handler stores in work byte 9. */
extern u8 func_800AFF98(void); /* @source 0x800AFF98 @kind function */

/* @source 0x801EF544
 * @behavior Work byte 2 step handler of this overlay's work object, entry 0 of
 *           the four-entry work byte 2 handler table at 0x801EEC14 (0x801EF544,
 *           0x801EF58C, 0x801EF648, 0x801EF6AC) that the sibling
 *           dispatchWorkByte2HandlerThenInvokeD290WhenWorkByte0Nonzero
 *           BmagicMagic15403_801EF4B4 measured exact copies into its own frame
 *           and invokes with no arguments. It calls the shared main-executable
 *           helper at 0x800AFF98, which returns the battle-participant lookup
 *           byte for the current participant, and stores that byte into
 *           scratchpad work byte 9 through the pointer cell at 0x1F800044. It
 *           then re-reads the cell and increments work byte 2 at byte width
 *           (lbu / addiu 1 / sb), advancing the work byte 2 step index. The
 *           0x18-byte frame keeps only $ra at 0x10; the function takes no
 *           arguments, returns nothing and reads or writes no other state. The
 *           72-byte body is byte-identical (cmp of both payload spans) to the
 *           body at 0x801EF024 of emi/bmagic/magic161/03, whose 72-byte body
 *           measured exact as seedWorkByte9FromHelperAFF98AndAdvanceWorkByte2
 *           BmagicMagic16103_801EF024; that sibling supplied the statement
 *           order, the byte local holding the helper result and the
 *           declaration style, and this span supplies its own callee and
 *           cell addresses, which are the same shared addresses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkByte9FromHelperAFF98AndAdvanceWorkByte2BmagicMagic15403_801EF544(void) {
  u8 value;

  value = func_800AFF98();
  D_1F800044[9] = value;
  D_1F800044[2]++;
}
