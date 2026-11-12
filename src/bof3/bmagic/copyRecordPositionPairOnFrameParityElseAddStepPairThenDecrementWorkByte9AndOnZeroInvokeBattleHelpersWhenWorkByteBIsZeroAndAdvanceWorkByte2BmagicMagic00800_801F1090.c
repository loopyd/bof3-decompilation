#include "bof3/bof3.h"

/* Shared frame counter in main RAM, outside this overlay's image. The shared
 * map owns the address (config/targets/shared/symbols.txt) and the exact
 * sibling lift at 0x801F0FA4 reads the same word, its low bit as a per-frame
 * phase. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The body reads the record's position word pair 0x34/0x38, the pair the exact
 * sibling lift of 0x801F0FA4 copies into a work object. Splat binds the address
 * through this target's generated undefined_syms_auto.txt, so no target-map row
 * is added here. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, the same named cell this target's exact
 * lifts advanceWorkByte2WhenHelperReadyBmagicMagic00800_801F117C and
 * subtractWorkColorTripletBy3ThenAdvanceWorkByte2WhenChannelIs80
 * BmagicMagic00800_801F11C4 declare. The named symbol lets the assembler fold
 * the constant address into each load (lui $v1,0x1F80 / lw $v1,0x44($v1)), and
 * the original re-reads the cell after every call and after the store groups
 * that may alias the cell object. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle byte in main RAM, outside this overlay's image: the byte the
 * zero-countdown arm forwards to the helper at 0x800A9820. Splat binds the
 * address through this target's generated undefined_syms_auto.txt. */
extern u8 D_80146384; /* @source 0x80146384 @kind data */

/* Shared helper in the main executable at 0x800AFC98, outside this overlay's
 * image: invoked with the constant pair (2, 4), the same selector/value pair
 * the exact lift invokeBattleHelpersSeedWorkWordsAndAdvanceWorkByte2
 * BmagicMagic00100_801EF164 passes. Splat binds the address through this
 * target's generated undefined_funcs_auto.txt, so no target-map row is added. */
void func_800AFC98(s32 arg0, s32 arg1);

/* Shared helper in the main executable at 0x800A9820, outside this overlay's
 * image: this body forwards the battle byte 0x80146384 to it. Splat binds the
 * address through this target's generated undefined_funcs_auto.txt. */
void func_800A9820(u8 arg0);

/* @source 0x801F1090
 * @behavior Entry 2 (index 2) of the six-entry code pointer table at
 * 0x801EEC94 (0x801F0E54, 0x801F0FA4, 0x801F1090, 0x801F117C, 0x801F11C4,
 * 0x801F1274), which 0x801F0DA8 copies into its own frame and selects with
 * scratchpad work-object byte 2, so this body runs while work byte 2 is 2. Bit
 * 0 of the shared frame counter at 0x80143E6C picks the arm. On a set bit the
 * handler reads the shared battle work-record pointer cell at 0x801EB4E0 and
 * the work-object pointer cell at 0x1F800044 once each and copies the record's
 * position word pair 0x34/0x38 into the work object's same pair, the second
 * store scheduled into the countdown branch delay slot; it then counts the work
 * object's countdown byte 9 down in place (lbu / addiu -1 / sb) and tests the
 * stored byte truncated to eight bits (andi $v0,$v0,0xFF / bnez). When that
 * countdown reaches zero it re-reads the pointer cell and, only while work byte
 * 0xB is still zero, invokes the shared helper 0x800AFC98 with the pair (2, 4)
 * and then forwards the shared battle byte 0x80146384 to the helper
 * 0x800A9820; it re-reads the pointer cell once more and increments work byte 2
 * with a byte-width add, moving dispatch to entry 3 at 0x801F117C. On a clear
 * bit it instead accumulates the object's step pair: word 0x34 gains step word
 * 0xC and word 0x38 gains step word 0x10, each read and written in place with
 * lw / addu / sw through one pointer-cell read. The 0x18-byte frame only keeps
 * $ra at 0x10 across the two calls; the function takes no arguments, returns
 * nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyRecordPositionPairOnFrameParityElseAddStepPairThenDecrementWorkByte9AndOnZeroInvokeBattleHelpersWhenWorkByteBIsZeroAndAdvanceWorkByte2BmagicMagic00800_801F1090(
    void) {
  if (D_80143E6C & 1u) {
    u8 *record = D_801EB4E0;
    u8 *work = g_battle_work;
    *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34);
    *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38);
    if (--work[9] == 0) {
      if (g_battle_work[0xB] == 0) {
        func_800AFC98(2, 4);
        func_800A9820(D_80146384);
      }
      g_battle_work[2]++;
    }
  } else {
    u8 *record = D_801EB4E0;
    u8 *work = g_battle_work;
    *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34) + *(s32 *)(work + 0xC);
    *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38) + *(s32 *)(work + 0x10);
  }
}
