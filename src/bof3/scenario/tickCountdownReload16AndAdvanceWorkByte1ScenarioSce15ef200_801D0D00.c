#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801D0D00
 * @behavior Entry 2 of this overlay's handler run at 0x801D1B1C, which the
 * overlay entry func_801D0C04 selects with the unsigned work byte at offset
 * +0x1 of the scratchpad work object published at pointer slot 0x1F800044 and
 * calls with no arguments: it ticks that object's countdown byte at +0x9 down
 * by one and, when the decremented byte reaches zero, re-arms it with 0x10 and
 * advances the same work byte at +0x1 by one so the next frame dispatches entry
 * 3 (0x801D0D5C). It takes no arguments, returns nothing and keeps no frame.
 * All three scratchpad accesses are separate loads of the 0x1F800044 cell
 * because the byte store through the loaded pointer may alias the cell.
 *
 * Evidence: the fourteen words from 0x801D1B1C name 0x801D0C70, 0x801D0CA0,
 * 0x801D0D00, 0x801D0D5C, 0x801D0DE8, 0x801D0E30, 0x801D0EC4, 0x801D0F68,
 * 0x801D0F9C, 0x801D1008, 0x801D1050, 0x801D10E8, 0x801D1118 and 0x801D11A4,
 * so this body is element 2 of that run.
 *
 * Shape evidence: its 21-instruction body is byte-identical to the tail of the
 * neighbouring span func_801D1118, which reaches the same statements behind a
 * prologue that keeps $ra across func_801D19A8; that span is still an `asm` row
 * with no source, so it is shape evidence rather than a clone source. The exact
 * lift tickCountdownReload128AndAdvanceWorkByte1ScenarioScena0300_801F7C50
 * (0x801F7C50 in emi/scenario/scena03/00) is the same leaf statement sequence
 * with 128 re-armed instead of 0x10, and the exact lift
 * invokeOriginHelperThenTickCountdownReload32AndAdvanceWorkByte1ScenarioSce15ef100_801D0D54
 * (0x801D0D54 in emi/scenario/sce15ef1/00) records the same `andi $v0,$v0,0xFF`
 * that the promoted u8 decrement produces.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownReload16AndAdvanceWorkByte1ScenarioSce15ef200_801D0D00(void) {
  if (--D_1F800044[9] == 0) {
    D_1F800044[9] = 0x10;
    D_1F800044[1]++;
  }
}
