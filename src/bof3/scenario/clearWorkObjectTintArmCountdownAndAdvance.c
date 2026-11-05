#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F7264
 * @behavior Handler for state 2 of the scratchpad work object per-frame
 * dispatch (the object byte at offset 0x01 selects the entry of the handler
 * table at 0x801FC980): it re-reads the work object pointer published at
 * 0x1F800044, zeroes the three color channels at 0x5D/0x5E/0x5F of that
 * object, stores 0xF0 in the object byte at 0x09 and moves the dispatch state
 * byte at 0x01 to 1; no other state is read or written. The name follows the
 * byte-identical sibling
 * clearWorkTintArmCountdownAndAdvanceWorkByte1ScenarioScena1300_801F6EE8
 * (0x801F6EE8, scena13/00), which differs only in its 0xF countdown
 * immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkObjectTintArmCountdownAndAdvance(void) {
  D_1F800044[0x5D] = D_1F800044[0x5E] = D_1F800044[0x5F] = 0;
  D_1F800044[9] = 0xF0;
  D_1F800044[1] = 1;
}
