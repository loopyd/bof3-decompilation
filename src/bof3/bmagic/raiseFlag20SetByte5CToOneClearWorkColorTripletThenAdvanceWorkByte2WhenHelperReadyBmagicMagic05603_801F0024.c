#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. Declared as the same volatile u8 pointer the exact bmagic siblings
 * use (copyRecordByte27ThenAdvanceWorkByte2BmagicMagic05603_801EFFE8 and
 * decrementWorkColorTripletBy16ThenCopyRecordTripletAndAdvanceWorkByte2When
 * ChannelIs80BmagicMagic05603_801F00C8): every statement that goes through it
 * re-reads the cell, because a store through the pointer it supplied may alias
 * the cell itself, and the original reloads it before each of the six store
 * groups. The named symbol lets the assembler fold the constant address into
 * each load (lui $vN,0x1F80 / lw $vN,68($vN)). */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Readiness poll helper of the main executable. The battle, boss, plchar and
 * bmagic lifts declare the same prototype, and the original keeps only the
 * returned byte as the ready test (jal then andi $v0,0xFF then beqz). */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC */

/* @source 0x801F0024
 * @behavior Fade-out step of the overlay work object. It calls the shared
 *           readiness helper func_8014DAEC and keeps only that helper's returned
 *           byte as a ready test: while the helper reports not ready the step
 *           writes nothing and returns. Once it reports ready the step reads the
 *           scratchpad work-object pointer cell at 0x1F800044 and raises bit 0x20
 *           of work byte 0, the flag the sibling decrementWorkCountdownThenSeed
 *           WorkTintFromByteBRefreshWorkFromRecordAndAdvanceWorkByte2Bmagic
 *           Magic01500_801EFFC0 and decrementWorkByte9ThenSetFlag20Mode1ClearTint
 *           TripletAndAdvanceWorkByte2BmagicMagic22203_801EF448 raise. It then
 *           re-reads the cell and sets work byte 0x5C to 1, re-reads the cell
 *           before each of the three colour channels it clears at 0x5D, 0x5E and
 *           0x5F, and re-reads the cell once more to advance work byte 2 by one
 *           (lbu / addiu 1 / sb), which moves the work-byte-2 dispatcher to the
 *           next step. Its address is word 1 of the nine-word code pointer run at
 *           0x801EEC10, which the step dispatcher func_801EFF28 copies into its
 *           0x40-byte frame and selects from with work byte 2 scaled by four, so
 *           the body runs as that dispatcher's step 1; the same word sits at
 *           0x801EEC14 of the twelve-word run based at 0x801EEC04, whose first
 *           three words the byte-1 dispatcher func_801EEC34 selects from. No jal
 *           in this payload names 0x801F0024, so it is reached through the run
 *           alone. The 0x18-byte frame only keeps the return address at 0x10
 *           across its single call; the step takes no arguments, returns
 *           nothing, and reads or writes no other state. The roles of work bytes
 *           0x5C, 0x5D, 0x5E and 0x5F are not established beyond the colour
 *           channels the sibling 801F00C8 fades at 0x5D, 0x5E and 0x5F, so the
 *           name describes the stores. The 0xA4-byte span is byte for byte the
 *           body of func_801EFF28 of emi/bmagic/magic001/00, whose map row is
 *           still asm, so no extracted payload carries a lifted copy; its store
 *           body alone is the exact sibling
 *           decrementWorkByte9ThenSetFlag20Mode1ClearTintTripletAndAdvanceWork
 *           Byte2BmagicMagic22203_801EF448 and its guard shape is the exact
 *           sibling requestResourceForWorkByte8ThenAdvanceWorkByte3WhenHelper
 *           ReadyPlcharPlp03400_801CF84C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag20SetByte5CToOneClearWorkColorTripletThenAdvanceWorkByte2WhenHelperReadyBmagicMagic05603_801F0024(void) {
  if (func_8014DAEC()) {
    g_battle_work[0] |= 0x20;
    g_battle_work[0x5C] = 1;
    g_battle_work[0x5D] = 0;
    g_battle_work[0x5E] = 0;
    g_battle_work[0x5F] = 0;
    g_battle_work[2]++;
  }
}
