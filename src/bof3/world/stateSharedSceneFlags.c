#include "bof3/bof3.h"

extern u8 D_80146875;

void func_80196070(void);

/* @source 0x801F2DAC
 * @behavior Entry 3 of this overlay's byte-1 handler run based at 0x801F37D0,
 * which dispatchByte1Table37d0 indexes with the
 * work record byte at +0x01: adds one to the shared scene state request byte
 * 0x80146875 and then runs the shared work-flag reset func_80196070, which
 * clears the scratchpad work bytes at offsets 0x00 through 0x04 and so ends
 * the run. Takes no arguments and returns nothing; the 0x18-byte frame only
 * keeps $ra across that call and the store of the incremented byte fills the
 * call delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateSharedSceneFlags(
    void) {
  u8 *state = &D_80146875;

  *state = *state + 1;
  func_80196070();
}
