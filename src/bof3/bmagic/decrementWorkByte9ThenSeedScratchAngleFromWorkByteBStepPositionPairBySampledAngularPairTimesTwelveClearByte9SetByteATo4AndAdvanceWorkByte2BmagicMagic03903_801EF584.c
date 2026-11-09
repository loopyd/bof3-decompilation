#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, already bound at that
 * address by this target's map as g_battle_work, the name the exact lifts
 * add16ToWorkByte9AndAdvanceWorkByte2At80BmagicMagic03903_801EF690 and
 * add8ToWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndInvoke
 * HelperWhenZeroBmagicMagic03903_801EF6E0 declare the same way. The original
 * re-materialises the cell with its own lui 0x1F80 plus lw 68($vN) pair at each
 * access group, because a store through the loaded pointer may alias the cell,
 * so the cell is named rather than written as a bare scratchpad constant. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay image.
 * This target's generated undefined_syms_auto.txt already binds the address
 * because the remaining assembly spans func_801EEC94, func_801EEEA8,
 * func_801EF070, func_801EF44C and func_801EF758 load it, so the name needs no
 * map row, and the exact sibling add8ToWorkByte9AndDecrementWorkByteAThen
 * DecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic03903_801EF6E0
 * declares it the same way. Words 0x34, 0x38 and 0x3C read here are the
 * position triplet the exact bmagic sibling decrementWorkByte9ThenCopyRecord
 * TripletAdd800000ToWord3CSetDirectionAndAngleToward8014638CRearmByte9And
 * AdvanceWorkByte2BmagicMagic00603_801EEF28 copies out of the record it reads
 * through 0x8014638C: the record layout is shared and the address is not this
 * target's own. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Overlay-local angular samplers: both take a 12-bit angle argument and return
 * one axis sample in the fixed-point scale this body multiplies by 0xC000 and
 * shifts right 12. The pair is the same one the exact bmagic sibling advance
 * WorkByteAStepWorkPositionPairByAngleWord0CAddWord20ToWord14AccumulateInto
 * Halfword3ECountdownWorkByte9AdvanceWorkByte2OnZeroAndDrawSpriteMode1Bmagic
 * Magic01200_801EF3C4 calls, and this repository documents the pair only as
 * "shared angular helpers" without tying either address to one axis, so the
 * declared parameter keeps its address-anchored spelling. The first answers
 * into the word 0x34 axis, the second, re-reading the seeded halfword, into
 * word 0x38. */
extern s32 func_801783C8(s32 angle);
extern s32 func_801782FC(s32 angle);

/* Scratchpad halfword at 0x1F80000A that carries the angle seeded from work
 * byte 0xB. The original builds its address once into $s0 (lui 0x1F80 /
 * addiu 0xA) because the value has to survive the func_801783C8 call, then
 * stores it with sh and re-reads it with lh for func_801782FC, so the body
 * keeps one pointer local instead of naming the cell at each use. Remaining
 * assembly spans func_801EF758 and func_801EFAA4 of this target load the same
 * address, so the generated undefined_syms_auto.txt already binds it and the
 * name needs no map row. */
extern s16 D_1F80000A; /* @source 0x1F80000A @kind unknown */

/* @source 0x801EF584
 * @behavior Per-frame handler entered through this target's tabledata segment:
 *           its address is the word at 0x801EEC28, entry index 9 of the
 *           twelve-word code-pointer run 0x801EEC04..0x801EEC34. The only
 *           in-payload reader of that run,
 *           dispatchWorkByte1HandlerBmagicMagic03903_801EEC34, copies its first
 *           three words into a 0x28-byte frame and indexes them with work byte
 *           0x1, and no jal word of the payload targets 0x801EF584, so this
 *           entry is selected from outside the payload. It reads the scratchpad
 *           work object through the pointer cell at 0x1F800044, the load
 *           issued before the 0x18-byte frame is created, and subtracts one
 *           from its unsigned byte 9 at byte width (lbu / addiu -1 / sb); the
 *           stored byte is zero-extended with andi $v0,$v0,0xFF and tested, so
 *           the truncated register rather than a reload supplies the compared
 *           value, and the rest of the body runs only when that countdown
 *           reaches zero. On zero it re-reads the cell, reads unsigned byte
 *           0xB, scales it by 0x200 into the scratchpad halfword 0x1F80000A
 *           (sll 9 / sh, the store filling the call delay slot) and passes the
 *           sixteen-bit value of that product, sign-extended with sll 25 / sra
 *           16, to the angular sampler func_801783C8. It then rewrites the work
 *           object's word 0x34 as the shared record's word 0x34, read through
 *           the 0x801EB4E0 cell, plus twelve times that answer, written as the
 *           12-bit fixed-point product (sll 1 / addu / sll 14 / sra 12). It
 *           reloads the scratchpad halfword with lh for the second sampler
 *           func_801782FC and rewrites word 0x38 the same way from the shared
 *           record's word 0x38, then takes the shared record's word 0x3C into a
 *           local, clears work byte 9 through the same pointer load, re-reads
 *           the cell so the byte 0xA = 4 store goes through the fresh pointer,
 *           stores that local into the work object's word 0x3C, and finally
 *           re-reads the cell once more to increment the dispatch state byte
 *           0x2, which advances this overlay's per-frame work byte 2 handler
 *           sequence. Giving the two stepped words and the copied word their own
 *           pointer and value locals is what the original's two cell reloads
 *           and its store order show. The 0x18-byte frame keeps only $ra at
 *           0x14 and $s0 at 0x10, $s0 holding the 0x1F80000A address across both
 *           calls. Takes no arguments, returns nothing through the jr $ra at
 *           0x801EF688 and reads or writes no other state. The roles of work
 *           bytes 9, 0xB, 0xA and 0x2 and of the position words are not
 *           established beyond what this body does to them, so they stay
 *           offset-anchored, and so do the two sampler answers. The 268-byte
 *           span holds no byte-identical twin anywhere under out/binaries,
 *           neither raw nor with both jal immediates masked in the payload's
 *           little-endian byte order. The body contains three relocations, the
 *           two shared sampler calls and the 0x801EB4E0 cell, all already bound
 *           by this target's generated auto files.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSeedScratchAngleFromWorkByteBStepPositionPairBySampledAngularPairTimesTwelveClearByte9SetByteATo4AndAdvanceWorkByte2BmagicMagic03903_801EF584(
    void) {
  u8 *record;
  u8 *record2;
  u8 *work;
  u8 *work2;
  u8 *work3;
  s16 *cell;
  s32 angle;
  s32 sample34;
  s32 sample38;
  s32 z;

  if (--g_battle_work[9] == 0) {
    angle = g_battle_work[0xB] * 0x200;
    cell = &D_1F80000A;
    *cell = angle;
    sample34 = func_801783C8((s16)angle);
    record = D_801EB4E0;
    work = g_battle_work;
    *(s32 *)(work + 0x34) =
        *(s32 *)(record + 0x34) + ((sample34 * 0xC000) >> 12);
    sample38 = func_801782FC(*cell);
    record2 = D_801EB4E0;
    work2 = g_battle_work;
    *(s32 *)(work2 + 0x38) =
        *(s32 *)(record2 + 0x38) + ((sample38 * 0xC000) >> 12);
    z = *(s32 *)(record2 + 0x3C);
    work2[9] = 0;
    work3 = g_battle_work;
    *(s32 *)(work2 + 0x3C) = z;
    work3[0xA] = 4;
    g_battle_work[2]++;
  }
}
