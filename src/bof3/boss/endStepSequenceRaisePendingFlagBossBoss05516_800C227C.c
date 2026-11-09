#include "bof3/bof3.h"

void func_801E567C(void);

extern u16 D_801462E8; /* @source 0x801462E8 @kind unknown */
extern u16 D_801463C0; /* @source 0x801463C0 @kind unknown */
extern u16 D_801463BE; /* @source 0x801463BE @kind unknown */

/* @source 0x800C227C
 * @behavior Phase-advance helper of this overlay, called by
 * dispatchStateByte2ThenAdvanceOnState8BossBoss05516_800C20A4 once the work
 * record's byte +1 is exactly 8 and its step byte +2 is below three. Gated on
 * bit 0x4 of the shared battle flag halfword 0x801462E8: with the bit set and
 * the current battle id halfword 0x801463C0 equal to 0x81 it raises byte +0xB
 * of the scratchpad work object at 0x1F800044 (the pending flag the sibling
 * queueSlotThenRaisePendingFlagBossBoss05516_800C2340 also raises), stores 8
 * into the battle halfword 0x801463BE and writes step byte +2 = 3, one past the
 * last step index the caller accepts, so the step sequence ends there. On any
 * other battle id it calls the engine scratch-state reset 0x801E567C instead,
 * the same entry resetScratchStateBossBoss05516_800C2244 calls first. The
 * pointer cell at 0x1F800044 is read twice, once per store, through the
 * zero-based cell table so each read is a `lui` plus offset load: the store
 * through the loaded pointer may alias the cell. Nothing but $ra lives in the
 * 0x18-byte frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void endStepSequenceRaisePendingFlagBossBoss05516_800C227C(void) {
  u8** scratch = SPAD_PTR_TABLE(u8);

  if (D_801462E8 & 4) {
    if (D_801463C0 == 0x81) {
      scratch[0x11][0xB] = 1;
      D_801463BE = 8;
      scratch[0x11][2] = 3;
    } else {
      func_801E567C();
    }
  }
}
