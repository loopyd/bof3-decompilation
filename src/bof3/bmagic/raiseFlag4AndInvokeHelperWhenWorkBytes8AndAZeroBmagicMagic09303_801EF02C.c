#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. It sits outside this overlay
 * image, which the generated undefined_syms_auto.txt row of this target already
 * names, so the name needs no target-map row. The original reads it unsigned,
 * ors 0x4 and stores it back, the store filling the helper call's delay slot. */
extern u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EF02C
 * @behavior Entry 1 of the two-entry local handler pair built by
 *           dispatchWorkByte1LocalPairThenPublishEachOccupiedPoolRecordAndRunItsStepBmagicMagic09303_801EEC04
 *           in its own frame. It reads the scratchpad work-object pointer cell
 *           at 0x1F800044 once, loads the work object's word 0x08 and masks it
 *           with 0xFF00FF00, which keeps its bytes 0x8 and 0xA and ignores bytes
 *           0x9 and 0xB. When both tested bytes are zero it raises bit 0x4 of the
 *           shared battle global halfword at 0x801462E8 (unsigned halfword load,
 *           ori and halfword store) and then invokes the shared helper
 *           func_801E5988; otherwise it returns without touching either. It takes
 *           no arguments, returns nothing and writes no other state, and its
 *           0x18-byte frame only keeps $ra at 0x10 across that call. The pointer
 *           cell load and the mask's lui are scheduled ahead of the frame's store
 *           of $ra, and the halfword store fills the call's delay slot, so the
 *           flag raise is one base register held across the call rather than a
 *           re-materialised address. Statement order, operand forms and
 *           declaration style are taken from the family's exact lifts of the same
 *           raise-flag-4-and-invoke shape, notably
 *           forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic21803_801EEDB8,
 *           with this span's own literals (mask 0xFF00FF00, word 0x08, bit 0x4)
 *           and without the battle-byte forward its sibling performs. The mask
 *           occurs in no other span of this tree, so no byte-identical sibling
 *           exists. The boundary comes from this overlay's own Splat segment,
 *           whose next entry starts at 0x801EF080. Representation note: the
 *           direct `D_801462E8 |= 4` spelling measured 16/21 instructions
 *           (72.73%) because it folds the halfword address into the load's base
 *           and re-materialises it into $at for the store, while taking the cell
 *           through a local pointer keeps one base register live and fills the
 *           call's delay slot, which measured 21/21; a `volatile` view measured
 *           19/22 and blocked that delay-slot fill.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndInvokeHelperWhenWorkBytes8AndAZeroBmagicMagic09303_801EF02C(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if ((*(u32 *)(work + 0x8) & 0xFF00FF00u) == 0) {
    u16 *flags = &D_801462E8;

    *flags |= 4;
    func_801E5988();
  }
}
