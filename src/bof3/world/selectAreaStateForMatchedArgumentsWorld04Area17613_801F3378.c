#include "bof3/bof3.h"

/* Leading byte of the main-RAM game-mode record, which the original reads with
 * a single `lbu` at the record base; no other field of the record is used.
 * @source 0x80145E98 @kind unknown */
extern u8 D_80145E98;
/* Signed leading byte of the shared scenario state record that
 * emi/etc/game/00 declares as scenarioState. The original takes it with `lb`
 * and tests it against 8, so the cell keeps that signed byte width.
 * @source 0x80146870 @kind bss */
extern s8 D_80146870;
/* Shared state-entry selector: dispatchIndexedStateHandler indexes the
 * D_801C84BC handler table with this byte, and the world overlays publish
 * their next state entry here. The original stores it with one `sb`.
 * @source 0x801448EB @kind unknown */
extern u8 D_801448EB;

/* @source 0x801F3378
 * @behavior Selects the shared area state entry 0x801448EB from the signed
 *   scenario byte 0x80146870 once the overlay request matches: it returns 0
 *   unless both halfword arguments read 0x3F and 0x0E (each truncated with
 *   `andi 0xFFFF`, so both are u16) and the leading byte of the main-RAM
 *   game-mode record 0x80145E98 reads 1. On a match it stores 0x1A while the
 *   scenario byte is at least 8 and 6 otherwise, then returns 1. No calls, no
 *   frame, no stack use and a single `sb`.
 *
 * Name evidence: 0x801448EB is the state selector that etc/game/00 dispatches
 *   its D_801C84BC handler table through (dispatchIndexedStateHandler reads it
 *   as an s8 table index) and that the world overlays set as their state
 *   entry; 0x80146870 is the leading byte of the etc/game/00 scenario-state
 *   record, declared s8 at its original `lb` width in
 *   include/bof3/ui/shop00_internal.h; 0x80145E98 is the leading byte of the
 *   main-RAM game-mode record, read here with `lbu`. The two arguments carry no
 *   evidenced role, so the name keeps them structural.
 *
 * Shape: the 108-byte range is byte-identical across nine world04 area
 *   overlays (payload span 0x778..0x7E4 of areas 176, 178 through 185), so the
 *   body holds no address-specific immediate; `bin/harness lift clone` reports
 *   the same identity for the area177 representative. The third guard is the
 *   decisive shape: written as a positive block whose body holds the store
 *   select and its own `return 1`, with `return 0` after the block, it keeps
 *   the at-least-8 arm inline and puts the below-8 arm out of line behind `j`
 *   with an unfilled branch delay slot. An early `return 0` guard followed by
 *   the same select at function level measured 20/27 with selecting into an
 *   s8/s32/u8 local or a conditional expression written straight into the cell,
 *   25/27 for this nested guard with the arms swapped, and a default
 *   `D_801448EB = 6` before a conditional `= 0x1A` reached 108 bytes at 22/27
 *   through two stores to the shared selector, which is not memory-event
 *   equivalent.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 selectAreaStateForMatchedArgumentsWorld04Area17613_801F3378(u16 arg0,
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
