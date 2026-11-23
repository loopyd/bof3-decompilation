#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. The two
 * accesses inside the guard load the cell again because the byte store through
 * the loaded pointer may alias it. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared scenario progress byte of the game image at 0x80146864. No shipped
 * header declares it for this payload, so the prototype stays local, exactly as
 * in the sibling lifts of the same overlay family. */
extern u8 g_ScenarioProgress;

/* @source 0x801D1008
 * @behavior Entry 9 of the overlay's fourteen-word code-pointer run at
 * 0x801D1B1C, which the overlay entry func_801D0C04 selects with the unsigned
 * work byte at offset +0x1 of the scratchpad work object published at pointer
 * slot 0x1F800044 and calls with no arguments: only while the shared scenario
 * progress byte g_ScenarioProgress reads 9 does it re-arm that object's
 * countdown byte at +0x9 with 0x1E and then advance the same work byte at +0x1
 * by one, so the next frame dispatches entry 10 (0x801D1050). Both accesses
 * reload the pointer cell because the byte store through the loaded pointer may
 * alias it. It takes no arguments, returns nothing and keeps no frame.
 *
 * Evidence: the fourteen words from 0x801D1B1C name 0x801D0C70, 0x801D0CA0,
 * 0x801D0D00, 0x801D0D5C, 0x801D0DE8, 0x801D0E30, 0x801D0EC4, 0x801D0F68,
 * 0x801D0F9C, 0x801D1008, 0x801D1050, 0x801D10E8, 0x801D1118 and 0x801D11A4,
 * so this body is element 9 of that run, the word after entry 8 (0x801D0F9C)
 * and the word before entry 10 (0x801D1050). The +0x9 byte is the countdown
 * that entries 1, 2, 7, 8 and 10 of the same run tick and re-arm, and the +0x1
 * byte is the run's own selector. The guard reads the same shared progress byte
 * that entry 5 of this run (0x801D0E30) tests against 14, and the arm-then-
 * advance tail repeated the armCountdown10ClearScreenPairAndAdvanceWorkByte...
 * lift of 0x801D0F68, which measured exact, with 0x1E in place of its 0xA and
 * its two halfword clears dropped; the pointer-cell and progress declarations
 * are the ones the same target's lift of
 * tickMoveScreenPair
 * (0x801D0F9C, measured exact) uses.
 *
 * Shape: 0x48-byte body, no byte-identical twin. A payload scan of every
 * extracted bin for the whole 0x48-byte span returned this payload alone, so the
 * span is not a clone of any lifted sibling and the tail above is shape
 * evidence only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armAdvanceByteScenario(void) {
  if (g_ScenarioProgress == 9) {
    D_1F800044[0x9] = 0x1E;
    D_1F800044[0x1]++;
  }
}
