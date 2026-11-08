#include "bof3/bof3.h"

void func_801E5988(void);

extern u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x800C1D7C
 * @behavior Raises bit 0x4 of the shared battle global halfword at 0x801462E8
 * with a halfword read-modify-write and then invokes the shared helper at
 * 0x801E5988 with no arguments, unconditionally. This entry is the fourth code
 * pointer of the boss step table at 0x800C1804 (0x800C1B14, 0x800C1BF0,
 * 0x800C1CE4, 0x800C1D7C), which dispatchStateByte1Table1804BossBoss05116_800C1AA8
 * copies into its frame and indexes by the work record byte 1, so no jal
 * reaches it. The 0x18-byte frame only keeps $ra at 0x10 across the helper
 * call and the halfword address stays in one register across the
 * read-modify-write. Its 0x34 bytes are identical to the exact sibling
 * raiseFlag4AndInvokeHelperBmagicMagic00800_801EFCFC, which raises the same bit
 * of the same halfword and calls the same helper from the bmagic overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndInvokeHelperBossBoss05116_800C1D7C(void) {
  u16 *flags = &D_801462E8;

  *flags |= 4;
  func_801E5988();
}
