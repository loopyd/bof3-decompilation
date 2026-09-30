#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FC748
 * @behavior Entry 6 of the 16-word in-image callback table at 0x801FE434 (word
 * at 0x801FE44C) that the overlay dispatcher
 * dispatchRecordCallbackScenarioScena0900_801FC5E4 (0x801FC5E4) indexes with
 * byte 0x7A of a record and hands the shared word D_8014686C, so the overlay
 * reaches this handler only indirectly. It clears byte 0 of the scratchpad work
 * object whose pointer the cell at 0x1F800044 publishes (`lui $v0,0x1F80` then
 * `lw $v0,0x44($v0)` and `sb $zero,0x0($v0)`), runs the shared front-end
 * startup helper func_8015C088 and then requests primary scene state 0x0D with
 * secondary sub-state 5 by storing 0x0D in the shared state byte D_80146874 and
 * then 5 in the shared sub-state byte D_80146875. It ignores the record pointer
 * and the second argument its caller passes, takes no arguments of its own and
 * returns nothing; the 0x18-byte frame only keeps $ra across the call, each of
 * the two shared-byte stores materialises its constant in $v0 (`addiu
 * $v0,$zero,imm` feeding each `sb`) and the work-byte store is sunk into the
 * call's delay slot, which is how the psyq compiler places a preceding
 * independent byte store through an already-loaded pointer, the same sinking
 * the battle overlay's exact initScratchWorkState (0x801E3334) shows. Its 0x40
 * bytes keep the request shape of the exact per-record sibling
 * clearProgressThenRequestPrimaryState5Substate0ScenarioScena0900_801FC62C
 * (0x801FC62C, table entry 1), which clears the shared progress byte
 * g_ScenarioProgress instead of a work byte and requests state 5 with sub-state
 * 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte0ThenRequestPrimaryState0DSubstate5ScenarioScena0900_801FC748(
    void) {
  SPAD_PTR_SLOT(u8, 0x44u)[0] = 0;
  func_8015C088();
  D_80146874 = 0xD;
  D_80146875 = 5;
}
