#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers. Declared as the same volatile u8 pointer the exact sibling
 * subtractWorkColorTripletBy3ThenAdvanceWorkByte2WhenChannelIs80
 * BmagicMagic00800_801F11C4 uses: this original re-reads the cell before each of
 * its four groups, and the named map symbol lets the assembler fold the
 * constant address into each load (lui $v1,0x1F80 / lw $v1,68($v1)). */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The sibling lifts read the same record's byte 0xB (0x801F04DC stores 0xFF
 * there); splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F08C4
 * @behavior Entry 4 (last) of the five-entry work-state table at 0x801EEC70
 * (0x801F05FC, 0x801F0710, 0x801F07D4, 0x801F0864, 0x801F08C4), which 0x801F0568
 * copies into its frame and dispatches on scratchpad work-object byte 2. Per
 * frame it subtracts 3 from the work object's three colour channels at bytes
 * 0x5D/0x5E/0x5F - the tint triplet that state entry 0x801F05FC seeds with
 * -0x80 - then decrements the work object's countdown byte 9 with a byte-width
 * read-modify-write (lbu / addiu -1 / sb) and tests the truncated stored value
 * against zero (andi 0xFF / bnez), the same countdown group the exact sibling
 * addWorkColorTripletBy8ThenDecrementWorkByte9ThenDecrementGlobalWorkByteBAnd
 * AdvanceWorkByte2OnZeroBmagicMagic01703_801EF8B4 records. Only when the
 * countdown reaches zero does it store 0xFF into byte 0xB of the shared battle
 * work record read through 0x801EB4E0 and invoke the shared helper at
 * 0x801E5988, with that store filling the call delay slot. Each of the four
 * work-object accesses re-reads the pointer cell at 0x1F800044, so the cell is
 * loaded four times. The 0x18-byte frame only keeps $ra at 0x10 because of the
 * call; it returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void subtractWorkColorTripletBy3ThenDecrementWorkByte9SetGlobalWorkByteBToFFAndInvokeHelperWhenZeroBmagicMagic00800_801F08C4(void) {
  g_battle_work[0x5D] -= 3;
  g_battle_work[0x5E] -= 3;
  g_battle_work[0x5F] -= 3;
  if (--g_battle_work[9] == 0) {
    D_801EB4E0[0xB] = 0xFF;
    func_801E5988();
  }
}
