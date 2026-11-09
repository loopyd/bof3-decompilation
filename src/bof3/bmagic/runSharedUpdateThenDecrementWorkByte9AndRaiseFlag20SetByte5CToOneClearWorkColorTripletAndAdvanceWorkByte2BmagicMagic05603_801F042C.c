#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers, bound at 0x1F800044 by this target's map. It keeps the same volatile
 * pointer spelling as the exact bmagic siblings
 * raiseFlag20SetByte5CToOneClearWorkColorTripletThenAdvanceWorkByte2WhenHelper
 * ReadyBmagicMagic05603_801F0024 and
 * copyRecordByte27ThenAdvanceWorkByte2BmagicMagic05603_801EFFE8: every
 * statement that goes through it re-reads the cell, because a store through the
 * pointer it supplied may alias the cell itself. The named symbol lets the
 * assembler fold the constant address into each load (lui $vN,0x1F80 / lw
 * $vN,68($vN)). */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared no-argument update helper of the main executable. The bmagic and plchar
 * lifts declare the same prototype, and this caller keeps none of its answer:
 * the original runs jal then nop and overwrites $v0 with the countdown load. */
extern void func_8014D978(void); /* @source 0x8014D978 */

/* @source 0x801F042C
 * @behavior Countdown step of the overlay work object. It first runs the shared
 *           no-argument update helper func_8014D978 and discards that helper's
 *           answer, then reads the scratchpad work-object pointer cell at
 *           0x1F800044 and decrements its unsigned byte 9 in place (lbu / addiu
 *           -1 / sb / andi 0xFF / bnez), so the rest of the body runs only when
 *           that countdown reaches zero. On zero it re-reads the cell and raises
 *           bit 0x20 of work byte 0 (the same flag the exact siblings
 *           raiseFlag20SetByte5CToOneClearWorkColorTripletThenAdvanceWorkByte2
 *           WhenHelperReadyBmagicMagic05603_801F0024 and
 *           decrementWorkByte9ThenSetFlag20Mode1ClearTintTripletAndAdvanceWork
 *           Byte2BmagicMagic22203_801EF448 raise), re-reads the cell and sets
 *           work byte 0x5C to 1, re-reads the cell before each of the three
 *           colour-channel bytes it clears at 0x5D, 0x5E and 0x5F, and re-reads
 *           the cell once more to advance work byte 2 by one (lbu / addiu 1 /
 *           sb), which moves the work-byte-2 dispatcher to the next step. Each
 *           store re-materialises the cell because a store through the loaded
 *           pointer may alias it; the three zero stores leave their load delay
 *           slots unfilled, while the byte-0x5C store fills its slot with the
 *           constant 1. The 0x18-byte frame only keeps the return address at
 *           0x10 across its single call; the step takes no arguments, returns
 *           nothing and reads or writes no other state. Its address is word 5 of
 *           the nine-word code pointer run based at 0x801EEC10, which the step
 *           dispatcher func_801EFF28 copies into its 0x40-byte frame and selects
 *           from with work byte 2 scaled by four, so the body runs as that
 *           dispatcher's step 5; no jal in this payload names 0x801F042C, so it
 *           is reached through the run alone. The roles of work byte 9 and of
 *           work bytes 0x5C, 0x5D, 0x5E and 0x5F are not established beyond the
 *           colour channels the sibling 801F00C8 fades at 0x5D, 0x5E and 0x5F,
 *           so the name describes the stores. The 0xC0-byte span is not byte
 *           identical to any span of the 396 payloads under out/binaries, and
 *           its nearest exact sibling is the same target's 0x801F0024, whose
 *           store body matches statement for statement while its guard is the
 *           readiness poll of func_8014DAEC instead of this countdown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSharedUpdateThenDecrementWorkByte9AndRaiseFlag20SetByte5CToOneClearWorkColorTripletAndAdvanceWorkByte2BmagicMagic05603_801F042C(void) {
  func_8014D978();
  if (--g_battle_work[9] == 0) {
    g_battle_work[0] |= 0x20;
    g_battle_work[0x5C] = 1;
    g_battle_work[0x5D] = 0;
    g_battle_work[0x5E] = 0;
    g_battle_work[0x5F] = 0;
    g_battle_work[2]++;
  }
}
