#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);

/* @source 0x801FE198
 * @behavior Overlay per-record handler: it first re-arms the overlay effect
 * group for the ids 0, 1 and 2 through func_80166E88(0, 1, 2, 0), then calls
 * the helper at 0x801C1400 with the constant argument 0, and finally the per-id
 * helper func_801C187C with the constant argument 2. It takes no arguments,
 * returns nothing, reads no memory of its own and writes none; the 0x18-byte
 * frame only keeps $ra across the three calls, the first three arm immediates
 * are materialised in $a0/$a1/$a2 before the first jal and the fourth one in
 * that jal's delay slot, and each later argument is materialised in the delay
 * slot of its own call. The address is index 11 - the word at 0x801FE4B8,
 * payload offset 0x78B8, reading 0x801FE198 - of the 22-entry code-pointer run
 * at 0x801FE48C that runs to the word at 0x801FE4E0 and is followed by a null
 * word at 0x801FE4E4, so no jal targets the address and the run word is its
 * only image mention. dispatchCoordinateLookupHandlerScenarioScena0600_801FDD14
 * indexes that run and invokes the entry with jalr, but the four five-byte
 * records at 0x801FE478 only reach entries 0 through 3. The 60 bytes equal
 * those of the exact sibling armEffectGroupThenInvokeTwoHelpersScenarioScena0600_801FE1D4
 * word for word except the argument immediate at 0x801FE1C0, 0x02000424 here
 * against 0x01000424 there, which is the only difference from that body, so
 * only this function's own stored literal came from this span. The address
 * anchor keeps the name target-qualified: entry 0x801FE198 of scena02/00 holds
 * an unrelated two-helper body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeTwoHelpersScenarioScena0600_801FE198(void) {
  func_80166E88(0, 1, 2, 0);
  func_801C1400(0);
  func_801C187C(2);
}
