#include "bof3/bof3.h"

/* Leading byte of the main-RAM game-mode record, read here with a single `lbu`.
 * @source 0x80145E98 @kind unknown */
extern u8 D_80145E98;
/* Signed leading byte of the shared scenario-state record, taken with `lb` and
 * tested against 8.
 * @source 0x80146870 @kind unknown */
extern s8 D_80146870;
/* Shared front-end state-entry selector that the world overlays publish their
 * next state entry into and that etc/game/00's indexed dispatcher reads as a
 * handler-table index.
 * @source 0x801448EB @kind unknown */
extern u8 D_801448EB;

/* @source 0x801F3378
 * @behavior Overlay request gate that selects the shared front-end state entry
 *   0x801448EB: it returns 0 unless both zero-extended halfword arguments read
 *   0x3F and 0x0E (each truncated with `andi 0xFFFF`, so both are u16) and the
 *   leading byte of the main-RAM game-mode record 0x80145E98 reads 1. On a
 *   match it stores 6 while the signed scenario byte 0x80146870 is below 8 and
 *   0x1A otherwise, then returns 1. The original body funnels that select into
 *   one `sb`: 6 while the byte is below 8 and 0x1A otherwise, with the 0x1A
 *   `addiu` in the `j` delay slot and the below-8 arm out of line; every failing
 *   guard answers through the shared `jr $ra` with `addu $v0,$zero,$zero` in its
 *   branch delay slot. No calls, no frame and no other stores.
 *
 * Name evidence: 0x801448EB is the state selector that etc/game/00 dispatches
 *   its handler table through (dispatchIndexedStateHandler reads it as an s8
 *   table index) and that the world overlays set as their state entry; 0x80146870
 *   is the leading byte of the etc/game/00 scenario-state record, declared s8 at
 *   its original `lb` width in include/bof3/world/area01613_internal.h and
 *   include/bof3/ui/shop00_internal.h; 0x80145E98 is the leading byte of the
 *   main-RAM game-mode record, read here with `lbu`. The two arguments carry no
 *   evidenced role, so the name keeps them structural.
 *
 * Shape: the 108-byte span is byte-identical to the exact function in
 *   emi/world04/area176/13. Keep the select and its success return inside the
 *   positive game-mode guard, with the at-least-8 arm first. This preserves the
 *   original's unfilled branch delay slot and the `j` over the below-8 arm;
 *   a function-level select after an early failing guard loses both.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 selectAreaStateForMatchedArgumentsWorld04Area18013_801F3378(u16 arg0,
                                                                u16 arg1) {
  if (arg0 != 0x3F) {
    return 0;
  }
  if (arg1 != 0xE) {
    return 0;
  }
  if (D_80145E98 == 1) {
    if (D_80146870 >= 8) {
      D_801448EB = 0x1A;
    } else {
      D_801448EB = 6;
    }
    return 1;
  }
  return 0;
}
