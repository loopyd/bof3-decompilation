#include "bof3/bof3.h"

/* Overlay-resident bytes the pair of record halfwords is restored from. The
 * installer of this trio, func_800C2184 (0x800C2184, not yet lifted), saves the
 * low byte of each halfword it overwrites into these two cells as it replaces
 * the record's 0x80145F18/0x80145F1A pair with the 0x80145F20/0x80145F22 pair,
 * so both cells sit in the zero tail of this payload's D_800C3174 table run and
 * start at zero. Nothing else in the payload reads or writes them. */
extern u8 D_800C32B4; /* @source 0x800C32B4 @kind unknown */
extern u8 D_800C32B8; /* @source 0x800C32B8 @kind unknown */

/* The shared battle step byte this overlay's other step handler func_800C2D7C
 * also writes (0x0F, 7 and 0x14 there) immediately before the same mode-tuple
 * call, and the two halfwords of the 0x140-stride local work record based at
 * 0x80145E90 that this handler restores (record offsets 0x88 and 0x8A). */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */
extern u16 D_80145F18; /* @source 0x80145F18 @kind unknown */
extern u16 D_80145F1A; /* @source 0x80145F1A @kind unknown */

/* Shared mode-tuple service of the main program; the in-repo lifts of the boss
 * overlays record it as seeding the three-byte battle mode tuple at 0x801462E0
 * with 5, 3 and 0, the tuple the 0x80146875 step byte accompanies. */
void initModeTuple530(void); /* @source 0x801DDF28 */

/* @source 0x800C2634
 * @behavior First slot of the handler trio this overlay's installer
 * func_800C2184 publishes into the shared battle hook slots 0x801463A4,
 * 0x801463A8 and 0x801463AC, and so the step handler the battle step calls each
 * frame. It stores 4 into the shared step byte D_80146875, restores the local
 * work record's halfword pair 0x80145F18 and 0x80145F1A from the two
 * overlay-resident bytes 0x800C32B4 and 0x800C32B8, and then runs
 * initModeTuple530, which seeds the three-byte battle mode tuple with 5, 3 and
 * 0. Both loads of the overlay bytes precede the frame's $ra store, in source
 * order, and the step-byte store precedes both halfword restores; the body has
 * no flag test and no branch, so it reaches the tuple service on every call,
 * unlike the sibling mode-transition handlers of this archive family which
 * select the step value from a battle flag bit first. It takes no arguments,
 * keeps the family's 0x18-byte frame with $ra at 0x10 across the call, touches
 * $v0/$a0/$v1/$at only and returns nothing. The installer 0x800C2184 and the
 * three addresses it publishes appear as no word of this payload and no `jal`
 * reaches them, so the main program enters the overlay through that installer
 * and calls this handler through hook slot 0x801463A4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStepByte4RestoreSavedRecordHalfwordsAndInitModeTuple530BossBoss02513_800C2634(
    void) {
  D_80146875 = 4;
  D_80145F18 = D_800C32B4;
  D_80145F1A = D_800C32B8;
  initModeTuple530();
}
