#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);

/* @source 0x801FE1D4
 * @behavior Overlay per-record handler: it first re-arms the overlay effect
 * group for the ids 0, 1 and 2 through func_80166E88(0, 1, 2, 0), then calls
 * the helper at 0x801C1400 with the constant argument 0, and finally the per-id
 * helper func_801C187C with the constant argument 1. It takes no arguments,
 * returns nothing, reads no memory of its own and writes none; the 0x18-byte
 * frame only keeps $ra across the three calls, the first three arm immediates
 * are materialised in $a0/$a1/$a2 before the first jal and the fourth one in
 * that jal's delay slot, and each later argument is materialised in the delay
 * slot of its own call. The address is index 12 - the word at 0x801FE4BC,
 * payload offset 0x78BC, reading 0x801FE1D4 - of the 22-entry code-pointer list
 * at 0x801FE48C that runs to the word at 0x801FE4E0 and is followed by a null
 * word at 0x801FE4E4; func_801FDD14 selects the entry with the signed index
 * func_801A7AD8(0x801FE478, 4) returns over the four five-byte records at
 * 0x801FE478 and invokes it indirectly, so no in-image jal targets the address.
 * The list neighbours 11 (0x801FE198, id 2) and 13 (0x801FE210, ids 1 and 2)
 * run the same arm call and helper head with a different func_801C187C id, and
 * entry 10 at 0x801FE15C is byte-identical to this one, so the address anchor
 * keeps this name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeTwoHelpersScenarioScena0600_801FE1D4(void) {
  func_80166E88(0, 1, 2, 0);
  func_801C1400(0);
  func_801C187C(1);
}
