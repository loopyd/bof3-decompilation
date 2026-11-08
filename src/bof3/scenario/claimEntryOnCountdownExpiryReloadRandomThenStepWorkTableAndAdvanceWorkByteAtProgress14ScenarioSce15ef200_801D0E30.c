#include "bof3/bof3.h"
#include <rand.h>

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared scenario progress byte of the game image at 0x80146864. No shipped
 * header declares it for this payload, so the prototype stays local, exactly as
 * in the sibling lifts of the same overlay family. */
extern u8 g_ScenarioProgress;

/* Work-table stepper of this payload, lifted as
 * stepWorkTableEntriesArmAdvanceDecayThenDrawActiveScenarioSce15ef200_801D1624:
 * it walks the 0x10 entries of
 * six-byte stride of the work table at 0x800E4800, and for every entry whose
 * lead byte is non-zero it advances that entry's state byte at +0x2 - on the
 * state-0 arm it claims the entry by writing 4 to the entry bytes at +0x1 and
 * +0x3, its argument to the halfword at +0x4 and 1 to the state byte, on
 * state 1 it increments the state byte, and on state 2 it subtracts 8 from
 * that halfword and clears the lead byte once the result is negative.
 * It returns non-zero in the frames where at least one entry lead byte read
 * non-zero. Both in-payload call sites pass the literal 0xF0, so the argument
 * and the result are declared as full words and only the low byte of the result
 * is consumed. */
s32 stepWorkTableEntriesArmAdvanceDecayThenDrawActiveScenarioSce15ef200_801D1624(
    s32 entry_value);

/* Claim helper of this payload: it takes the first entry of the 0x800E4800 work
 * table whose lead byte reads zero, sets that lead byte to 1 and clears the
 * entry state byte at +0x2. It takes no arguments and returns nothing. */
void claimFreeWorkTableEntryAt800E4800ScenarioSce15ef200_801D15DC(void);

/* @source 0x801D0E30
 * @behavior Entry 1 of this overlay step handler run at 0x801D1B2C, which is
 * word 5 of the fourteen-word code-pointer run at 0x801D1B1C and which
 * dispatchWorkByte1HandlerScenarioSce15ef200_801D0DA4 selects with the unsigned
 * work byte at offset +0x1 of the scratchpad work object published at pointer
 * slot 0x1F800044: it reads that object's countdown byte at +0x9, and when the
 * byte reads zero it claims a work-table entry through
 * claimFreeWorkTableEntryAt800E4800ScenarioSce15ef200_801D15DC and re-arms the
 * countdown with the low nibble of rand() plus 0x18, while a non-zero countdown
 * only ticks down by one. Both arms then store the byte at +0x9 through one
 * shared base, so the load, the decrement and the re-arm all end in a single
 * store placed after the join. It then runs the work-table stepper
 * stepWorkTableEntriesArmAdvanceDecayThenDrawActiveScenarioSce15ef200_801D1624
 * with the literal 0xF0 (the value that stepper stores as the halfword at +0x4
 * of every entry it claims in this frame) and, when the shared scenario progress
 * byte g_ScenarioProgress reads 14, advances the work byte at +0x1 by one so the
 * next frame of that run dispatches entry 2 (0x801D0EC4). It takes no
 * arguments, returns nothing and the 0x18-byte frame only keeps $ra at 0x10
 * across the three calls.
 *
 * Evidence: the fourteen words from 0x801D1B1C name 0x801D0C70, 0x801D0CA0,
 * 0x801D0D00, 0x801D0D5C, 0x801D0DE8, 0x801D0E30, 0x801D0EC4, 0x801D0F68,
 * 0x801D0F9C, 0x801D1008, 0x801D1050, 0x801D10E8, 0x801D1118 and 0x801D11A4,
 * so this body is element 5 of that run and element 1 of the step-handler run
 * based at 0x801D1B2C. The three calls are the jal at 0x801D0E50 to
 * 0x801D15DC, the jal at 0x801D0E58 to rand (0x8017E3D4, the same libc entry
 * the sibling lifts of this family call), and the jal at 0x801D0E7C to
 * 0x801D1624. The countdown byte at +0x9 is the same byte
 * armWorkHalfword2E300ClearCountdownAndWorkTableThenAdvanceWorkByte1ScenarioSce15ef200_801D0DE8
 * zeroes and tickCountdownReload16AndAdvanceWorkByte1ScenarioSce15ef200_801D0D00
 * decrements, and 0x80146864 is the shared progress byte the game image
 * symbols bind as g_ScenarioProgress. The claim helper's own lift records this
 * caller and the same re-arm, so the countdown and table roles agree from
 * caller and callee side.
 *
 * Shape evidence: the exact lift
 * seedWorkByteBRandomNibbleClearBytes9AThenAdvanceWorkByte2BmagicMagic14303_801EEF10
 * records the same rand() low-nibble reload behind a random arm, and the exact
 * lift addRandomByteToWorkHalfword58AndAdvanceAtProgress11ScenarioScena0800_801F7200
 * records the same `g_ScenarioProgress == <literal>` guard that advances the
 * work byte at +0x1. The 0x94-byte span has no byte-identical sibling in the
 * tree, so no clone applies.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimEntryOnCountdownExpiryReloadRandomThenStepWorkTableAndAdvanceWorkByteAtProgress14ScenarioSce15ef200_801D0E30(void) {
  s32 random;
  u8* work;

  work = D_1F800044;
  if (work[9] == 0) {
    claimFreeWorkTableEntryAt800E4800ScenarioSce15ef200_801D15DC();
    random = rand() & 0xF;
    work = D_1F800044;
    work[9] = random + 0x18;
  } else {
    work[9]--;
  }
  stepWorkTableEntriesArmAdvanceDecayThenDrawActiveScenarioSce15ef200_801D1624(0xF0);
  if (g_ScenarioProgress == 0xE) {
    D_1F800044[1]++;
  }
}
