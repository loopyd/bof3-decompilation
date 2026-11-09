#include "bof3/bof3.h"

extern u8 D_801448EB; /* @source 0x801448EB @kind data */
extern u8 D_801448EC; /* @source 0x801448EC @kind data */

void func_8015C088(void);

/* @source 0x801F315C
 * @behavior Overlay entry that starts shared state entry 0x39 at progress 0x0A
 *           when either packed argument matches. It returns 1, and on the way
 *           runs the shared front-end startup helper func_8015C088 and stores
 *           0x39 into the state selector D_801448EB and 0x0A into the progress
 *           byte D_801448EC, whenever the first argument or the second argument
 *           equals 0x18000 or 0x338000; otherwise it returns 0 with no call and
 *           no store. Both literals are the exact packed words the exact
 *           sibling startStateEntry3AProgress0AWhenPackedArgRangesWorld03Area15013_801F2F58
 *           range-tests through the high half of the same two-word argument
 *           pair, and both carry 0x8000 in the low half, so the arguments keep
 *           their structural packed-word role. The 0x18-byte frame keeps only
 *           $ra, the $ra store fills the first test's branch delay slot, each
 *           byte store materialises the 0x8014 page in $at on its own with its
 *           own constant in $v1, and the match arm sets the 1 in $v0 and falls
 *           into the single shared epilogue while the zero-return arm branches
 *           to that epilogue with the zero in the last test's delay slot.
 *
 * Name evidence: 0x801448EB is the state selector game00 dispatches the
 *           handler table 0x801C84BC through (dispatchIndexedStateHandler reads
 *           it as an s8 table index) and that this tree's world overlays set as
 *           their state entry, and 0x801448EC is the progress byte the same
 *           overlays store beside it, in the wording of the accepted exact
 *           sibling startStateEntry3AProgress0AWhenPackedArgRangesWorld03Area15013_801F2F58,
 *           which calls the same helper and writes the same byte pair. The
 *           sibling in this target, seedStateEntry39Progress14AndClearEffectBankIfSharedFlag8AElseRequestKind1World04Area19313_801F31C0,
 *           carries the same 0x39 state-entry value. No word of this payload
 *           holds 0x801F315C, so no jal and no table entry names the caller and
 *           the two argument roles stay structural.
 *
 * Shape: no payload under out/binaries holds this body's opening words anywhere
 *           but this span, so the range has no byte-identical twin and no clone
 *           source. The statement order follows the accepted exact siblings of
 *           this family, and this span's own constants, stores and helper come
 *           from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 startStateEntry39Progress0AWhenEitherPackedWordMatches18000Or338000World04Area19313_801F315C(
    s32 arg0, s32 arg1) {
  if (arg0 == 0x18000 || arg1 == 0x18000 || arg0 == 0x338000 ||
      arg1 == 0x338000) {
    func_8015C088();
    D_801448EB = 0x39;
    D_801448EC = 0x0A;
    return 1;
  }
  return 0;
}
