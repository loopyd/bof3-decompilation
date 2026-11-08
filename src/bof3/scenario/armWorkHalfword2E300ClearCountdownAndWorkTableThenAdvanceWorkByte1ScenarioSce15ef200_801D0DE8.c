#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void clearWorkTableAt800E4800ScenarioSce15ef200_801D15B0(void);

/* @source 0x801D0DE8
 * @behavior Entry 0 of this overlay step handler run at 0x801D1B2C, which is
 * word 4 of the fourteen-word code-pointer run at 0x801D1B1C and which
 * dispatchWorkByte1HandlerScenarioSce15ef200_801D0DA4 selects with the
 * unsigned work byte at offset +0x1 of the scratchpad work object published at
 * pointer slot 0x1F800044. It arms that object with no arguments by storing
 * 0x12C in its halfword at +0x2E, zeroes its countdown byte at +0x9, runs the
 * overlay helper that clears the lead byte of every 6-byte entry of the
 * 0x800E4800 work table (jal at 0x801D0E00), and then advances the same work
 * byte at +0x1 by one, so the next frame dispatches entry 1 (0x801D0E30). The
 * halfword arm and the countdown clear share one load of the scratchpad
 * pointer cell, while the post-call advance re-reads that cell, because the
 * call may rewrite it. It takes no arguments and returns nothing; the
 * 0x18-byte frame only keeps $ra at 0x10, and the call delay slot carries the
 * countdown clear. Field roles beyond the countdown byte stay offset-anchored.
 *
 * Evidence: the fourteen words from 0x801D1B1C name 0x801D0C70, 0x801D0CA0,
 * 0x801D0D00, 0x801D0D5C, 0x801D0DE8, 0x801D0E30, 0x801D0EC4, 0x801D0F68,
 * 0x801D0F9C, 0x801D1008, 0x801D1050, 0x801D10E8, 0x801D1118 and 0x801D11A4,
 * and the only jal in this payload to 0x801D15B0 is the one at 0x801D0E00.
 *
 * Shape evidence: the exact lift
 * seedWorkObjectAndClearWorkTableThenAdvanceStateScenarioScena0100_801F73FC
 * (0x801F73FC in emi/scenario/scena01/00) is the same body with two extra word
 * stores and a sound cue, and it records the same arm-clear-call-advance order
 * with one pointer local per original cell read; the exact lift
 * tickCountdownReload16AndAdvanceWorkByte1ScenarioSce15ef200_801D0D00 in this
 * target records the scratchpad pointer spelling and the countdown byte at
 * +0x9. The 0x48-byte span has no byte-identical sibling in the tree, so no
 * clone applies.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armWorkHalfword2E300ClearCountdownAndWorkTableThenAdvanceWorkByte1ScenarioSce15ef200_801D0DE8(void) {
  u8* work;

  work = D_1F800044;
  *(u16*)(work + 0x2E) = 0x12C;
  work[9] = 0;
  clearWorkTableAt800E4800ScenarioSce15ef200_801D15B0();
  D_1F800044[1]++;
}
