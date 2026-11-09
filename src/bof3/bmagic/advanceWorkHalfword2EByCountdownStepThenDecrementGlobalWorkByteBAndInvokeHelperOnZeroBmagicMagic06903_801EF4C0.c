#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. This target's map binds it at 0x1F800044 as g_battle_work, the name
 * this target's other lifts already use, so the symbolic address folds into the
 * single `lui $a0,0x1F80` / `lw $a0,0x44($a0)` pair the body issues before it
 * creates its frame, and no second read of the cell is needed: every later
 * access goes through the loaded pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the overlay's remaining assembly loads it, so the name needs no map row, and
 * every access here is byte-addressed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF4C0
 * @behavior Work-byte-1 state 9 handler of this overlay: the last of the ten
 *           entries of the handler table based at 0x801F04F8 that the byte 1
 *           dispatcher at 0x801EEE90 indexes with the scratchpad work object's
 *           state byte 0x1, and the only thing in the payload that addresses
 *           this function, so it has no jal caller. It reads the work object
 *           through the pointer cell at 0x1F800044 once, loads the object's
 *           countdown word 0xC and its accumulated halfword 0x2E, and stores
 *           the word less one back to word 0xC. It then reads word 0xC again
 *           because the halfword store may alias the cell, forms the sum of the
 *           loaded halfword and the decremented word, and tests that re-read
 *           word. Only when the countdown reached zero does the body run: it
 *           reads the shared battle work record pointer at 0x801EB4E0,
 *           decrements that record's byte 0x0B by one at byte width (lbu /
 *           addiu -0x1 / sb) and invokes the shared helper at 0x801E5988, the
 *           store filling the call delay slot. The sum store fills the branch
 *           delay slot, so it executes on both paths. The 0x18-byte frame only
 *           keeps $ra at 0x10 across the call, and the cell load is issued
 *           before the frame is created because nothing before it needs the
 *           stack. Takes no arguments, returns nothing through the jr $ra at
 *           0x801EF518 and reads or writes no other state. The roles of word
 *           0xC, halfword 0x2E and bytes 0x0B stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkHalfword2EByCountdownStepThenDecrementGlobalWorkByteBAndInvokeHelperOnZeroBmagicMagic06903_801EF4C0(
    void) {
  u8 *work;
  u32 countdown;
  u16 step;

  work = g_battle_work;
  countdown = ((u32 *)work)[3];
  step = ((u16 *)work)[0x17];
  countdown -= 1;
  ((u32 *)work)[3] = countdown;
  ((u16 *)work)[0x17] = step + countdown;
  if (((u32 *)work)[3] == 0) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
