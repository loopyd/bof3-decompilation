#include "bof3/bof3.h"

/* Shared frame counter in main RAM, outside this overlay's image. The shared
 * map owns the address (config/targets/shared/symbols.txt) and the exact sibling
 * copyWorkCursorPairWithFrameParityDeltaThenCountdownWorkByte9ResetTo8And
 * DecrementWorkByte2BmagicMagic22003_801EF40C reads the same word, its low bit
 * as a per-frame phase. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The body reads the record's position word pair 0x34/0x38, the pair the exact
 * sibling lifts of the battle overlays copy into a work object. Splat binds the
 * address through this target's generated undefined_syms_auto.txt, so no
 * target-map row is added here. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, the same named cell this target's exact
 * lifts subtractWorkColorTripletBy3ThenDecrementWorkByte9SetGlobalWorkByteBToFF
 * AndInvokeHelperWhenZeroBmagicMagic00800_801F08C4 and
 * advanceWorkByte2WhenHelperReadyBmagicMagic00800_801F117C declare. The named
 * symbol lets the assembler fold the constant address into each load
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)), and the original re-reads the cell after
 * every store made through the pointer it supplied, because such a store may
 * alias the cell object. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared readiness helper of the battle overlays at 0x8014DAEC, outside this
 * overlay's image: the exact sibling advanceWorkByte2WhenHelperReady
 * BmagicMagic00800_801F117C tests its returned readiness byte, while this body
 * drops it and keeps only the call. Splat binds the address through this
 * target's generated undefined_funcs_auto.txt, so no target-map row is added. */
void func_8014DAEC(void);

/* @source 0x801F0FA4
 * @behavior Work-state handler of this overlay: entry 1 of the six-entry code
 * pointer table at 0x801EEC94 (0x801F0E54, 0x801F0FA4, 0x801F1090, 0x801F117C,
 * 0x801F11C4, 0x801F1274), which 0x801F0DA8 copies into its frame and selects
 * with scratchpad work-object byte 2, so this body runs while work byte 2 is 1.
 * Bit 0 of the shared frame counter at 0x80143E6C picks the arm. On a set bit
 * the handler copies the shared battle work record's position word pair
 * 0x34/0x38 into the work object's same pair, both stores issued through one
 * pointer-cell read, then invokes the shared helper 0x8014DAEC and drops its
 * readiness byte; on a clear bit it instead accumulates the object's step pair:
 * word 0x34 gains step word 0xC and word 0x38 gains step word 0x10, each read
 * and written in place with lw / addu / sw through one pointer-cell read. Only
 * on the set-bit arm does it count the object's countdown byte 9 down in place
 * (lbu / addiu -1 / sb) and test the stored byte truncated to eight bits
 * (andi $v0,$v0,0xFF / bnez); when that countdown reaches zero it re-reads the
 * pointer cell to store work byte 0xB plus 4 into byte 9, then re-reads the cell
 * once more to increment work byte 2 with a byte-width add, moving dispatch to
 * entry 2 at 0x801F1090. The 0x18-byte frame only keeps $ra at 0x10 across the
 * one call; the function takes no arguments, returns nothing and reads or writes
 * no other state.
 *
 * Representation notes: the first seed reused one function-scope work-object
 * base for the post-call reseed and measured 51 of 59 instructions, the cell
 * re-read into a second base (lui/lw the cell twice) where the original reads
 * it once, and 55 of 59 once that reseed took its own base; both arms owning
 * their record and work bases as block-scope declarations with initializers is
 * what gave each arm the registers the payload rotates ($v1 for the record,
 * $a0 for the work object).
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyRecordPositionPairOnFrameParityElseAddStepPairThenInvokeHelperDecrementWorkByte9AndOnZeroReseedWorkByte9FromWorkByteBPlus4AndAdvanceWorkByte2BmagicMagic00800_801F0FA4(
    void) {
  if (D_80143E6C & 1u) {
    u8 *record = D_801EB4E0;
    u8 *work = g_battle_work;
    *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34);
    *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38);
    func_8014DAEC();
    if (--g_battle_work[9] == 0) {
      u8 *reseed;
      reseed = g_battle_work;
      reseed[9] = reseed[0xB] + 4;
      g_battle_work[2]++;
    }
  } else {
    u8 *record = D_801EB4E0;
    u8 *work = g_battle_work;
    *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34) + *(s32 *)(work + 0xC);
    *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38) + *(s32 *)(work + 0x10);
  }
}
