#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. It sits outside this overlay
 * image, and this target's generated undefined-symbol table already binds the
 * address (D_801462E8 = 0x801462E8), so the cell needs no target-map row. The
 * sibling battle lifts bind the same word the same way. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEFA8
 * @behavior Byte-1 step body of this overlay: it raises bit 0x4 of the shared
 *           battle global halfword at 0x801462E8 with an unsigned halfword
 *           read-modify-write and then invokes the helper at 0x801E5988 with no
 *           arguments. One register carries the halfword address through the
 *           load and the store, so the read-modify-write is written through a
 *           local pointer, and the halfword store fills that call's delay slot.
 *           Its address is the third word (index 2) of the nine-word
 *           code-pointer run at 0x801EEC04, whose words are 0x801EED30,
 *           0x801EEF38, 0x801EEFA8, 0x801EF0BC, 0x801EF104, 0x801EF208,
 *           0x801EF260, 0x801EF2CC and 0x801EF328; the dispatcher at
 *           0x801EEC28 copies the first three of those words into its frame and
 *           calls the copy that the work object's unsigned byte 0x01 selects
 *           with no bound. The 0x18-byte frame only keeps $ra at 0x10 across
 *           the helper call. Takes no arguments, returns no value and reads or
 *           writes no other state. A payload cmp over its 52 bytes proved this
 *           span byte-identical to the exact lifts
 *           raiseFlag4AndInvokeHelperBmagicMagic01500_801EF1CC,
 *           raiseFlag4AndInvokeHelperBmagicMagic01500_801EF9B0 and
 *           raiseFlag4AndInvokeHelperBmagicMagic05603_801EF17C, whose statement
 *           order, operand forms and declaration style this body reuses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4CallStep(void) {
  u16 *flags = (u16 *)&D_801462E8;

  *flags |= 4;
  func_801E5988();
}
