#include "bof3/bof3.h"

u32 func_801F7748(void);
void func_80196070(void);

/* @source 0x801F717C
 * @behavior Takes no arguments and returns nothing: calls the overlay-local work-table sweeper at
 * 0x801F7748, masks its result to the low byte and calls the shared scratchpad work-state reset
 * helper at 0x80196070 only when that low byte is zero. The sweeper walks the 0x80 entries of the
 * 0x14-byte-stride work table at 0x800E4800 and returns a flag word that is cleared at entry and
 * set to 1 as soon as it clears the lead byte of one of those entries, so this handler resets the
 * work object on the frames where no entry was cleared; func_80196070 zeroes work-object bytes
 * 0x00-0x04, including dispatch byte 1, returning the overlay to work state 0. It reads no state
 * of its own and writes none beyond what the two callees touch; the 0x18-byte frame only keeps $ra
 * across the calls and the second call is reached only on the zero path. The address is entry 4 of
 * this overlay's per-frame handler table at 0x801FE57C. Its 0x34 bytes are the same instruction
 * shape as the exact siblings resetWorkStateWhenNoEntryClearedScenarioScena0100_801F7644
 * (0x801F7644 in emi/scenario/scena01/00), resetWorkStateWhenNoRecordActiveScenarioScena0900_801F73F0
 * (0x801F73F0 in emi/scenario/scena09/00) and resetWorkFlagsOnZeroRefreshScenarioScena0800_801F7A9C
 * (0x801F7A9C in emi/scenario/scena08/00), which
 * differ only in the sweeper they call; the name reuses the first sibling's reviewed wording for
 * this exact role with this target's qualifier and address anchor.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetWorkStateWhenNoEntryClearedScenarioScena1500_801F717C(void) {
  if ((u8)func_801F7748() == 0) {
    func_80196070();
  }
}
