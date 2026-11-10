#include "bof3/bof3.h"

extern int rand(void);

/* Scratchpad work-object pointer cell at 0x1F800044, written with the name
 * every other lifted body of this overlay gives it. The cell holds the record
 * pointer rather than the record. The original materialises it with its own lui
 * 0x1F80 plus lw 0x44 pair at every read, and this body spells the cell once per
 * read, so the six reads it makes stay six loads: a store through the pointer it
 * supplied may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad angle cell at 0x1F800004. The seed path stores work byte 0xB scaled
 * by 0x80 into it at word width and hands the same angle to the first rotator;
 * the cell keeps the angle the second rotator reads back, and the original folds
 * its base once into a saved register (lui $s1,0x1F80 / addiu $s1,$s1,4) rather
 * than re-materialising it at the two accesses, so the source keeps the address
 * in one local. The generated undefined_syms_auto.txt already binds the address,
 * so this lift adds no target-map row. */
extern u32 D_1F800004; /* @source 0x1F800004 @kind unknown */

/* Battle record pointer published at 0x8014638C in main RAM, outside this
 * overlay's 0x801EEC00-0x801F21E4 image; Splat binds the address through this
 * target's generated undefined_syms_auto.txt, so no target-map row is needed.
 * Its role is not proven, so the name stays address-anchored, the spelling the
 * other lifts of this overlay and of the neighbouring bmagic payloads use. Only
 * the record words 0x34, 0x38 and 0x3C are read here, once into a saved register
 * that stays live across all three calls. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* The overlay-local rotation result pair: the 0x1000-per-turn angle passed in
 * arg0 yields the local sine and cosine result, which the emitters elsewhere
 * scale by a radius and shift right. This body scales by nothing, so it hands
 * each rotator the angle word it published in the scratchpad cell. */
extern s32 func_801782FC(s32 arg0);
extern s32 func_801783C8(s32 arg0);

/* @source 0x801EFE80
 * @behavior Overlay work-handler step, entry 1 of the three-entry code-pointer
 *   run at 0x801F03D8 (0x801EFDF8, 0x801EFE80, 0x801EFF88) that the byte-1
 *   dispatcher at 0x801EFDB4 indexes with the scratchpad work object's unsigned
 *   byte 0x1, and the word the byte-2 dispatcher
 *   dispatchWorkByte2HandlerThenDrawAndPopWhenWorkBytes0And9SetBmagicMagic01303_801EFDF8
 *   reaches as index 0 of its base 0x801F03DC. The run's words are called
 *   through jalr with no arguments and no word in this payload addresses this
 *   one, so it takes no arguments and returns nothing.
 *   The body first reads the work-object pointer cell at 0x1F800044,
 *   decrements the record's unsigned byte 0x9 by one at byte width, masks the
 *   stored register with andi 0xFF and returns while that byte is not zero, so
 *   byte 0x9 is the frame countdown that gates the whole seed. The 0x20-byte
 *   frame keeps $ra at 0x18, $s1 at 0x14 and $s0 at 0x10, and the cell read for
 *   the countdown precedes frame creation.
 *   When the countdown reaches zero the seed path re-reads the cell and stores
 *   work byte 0xB scaled by 0x80 into the scratchpad angle cell at 0x1F800004,
 *   which turns the direction byte into the 0x1000-per-turn angle the rotators
 *   take. The battle record pointer at 0x8014638C is read into $s0 after that
 *   store and stays live across all three calls. The first rotator takes the
 *   angle word still in its argument register, and its result shifted left by
 *   eight and arithmetic-shifted right by nine, which keeps the low 24 bits and
 *   halves them, is added to work word 0x34 over one fresh read of the pointer
 *   cell. The second rotator takes the angle the scratchpad cell holds, so that
 *   access reads the cell rather than a local; the same shift and halving of its
 *   result is added to work word 0x38 over a second fresh read of the cell, and
 *   the record word 0x3C is copied into work word 0x3C through that same read.
 *   The remaining seed writes hold one pointer across three stores, because each
 *   store through it may alias the cell: work word 0x14 takes (rand() & 7) +
 *   0x80, work byte 0x9 is reset to 0x10 so the countdown restarts at sixteen
 *   frames, and work word 0x20 takes -0xC. The cell is read once more before
 *   that last store and held for the closing increment of work byte 0x2 by one
 *   at byte width, which is the handler index the byte-2 dispatcher reads, so
 *   the record leaves this step for the run's next entry.
 *   Reads no state outside the pointer cell, the scratchpad angle cell, the
 *   battle record pointer and rand(). The 264-byte body is not byte-identical to
 *   any span of the 396 shipped EMI payloads under out/binaries, so no clone or
 *   sibling seed applies; the neighbouring bmagic handlers seeded its statement
 *   order, its saved-register choices and its held-pointer spelling instead.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countDownWorkByte9ThenSeedAngleFromByteBAddHalvedRotatorStepsToWords34And38CopyWord3CSetWord14ToRandPlus128Word20ToMinus12ResetByte9To16AndAdvanceWorkByte2BmagicMagic01303_801EFE80(
    void) {
  u8 *next;
  u8 *record;
  u8 *battle;
  u8 *work;
  u8 *tail;
  u32 *angle_cell;
  s32 angle;
  s32 step;
  s32 jitter;

  if (--FIELD_REF(u8, g_battle_work, 9) == 0) {
    angle_cell = &D_1F800004;
    record = g_battle_work;
    angle = (s32)record[0xB] << 7;
    *angle_cell = (u32)angle;
    battle = D_8014638C;
    step = func_801782FC(angle);
    FIELD_REF(u32, g_battle_work, 0x34) =
        FIELD_REF(u32, battle, 0x34) + ((step << 8) >> 9);
    step = func_801783C8((s32)*angle_cell);
    work = g_battle_work;
    FIELD_REF(u32, work, 0x38) = FIELD_REF(u32, battle, 0x38) + ((step << 8) >> 9);
    FIELD_REF(u32, work, 0x3C) = FIELD_REF(u32, battle, 0x3C);
    jitter = rand();
    tail = g_battle_work;
    FIELD_REF(u32, tail, 0x14) = (jitter & 7) + 0x80;
    FIELD_REF(u8, tail, 9) = 0x10;
    next = g_battle_work;
    FIELD_REF(u32, tail, 0x20) = -0xC;
    FIELD_REF(u8, next, 2)++;
  }
}
