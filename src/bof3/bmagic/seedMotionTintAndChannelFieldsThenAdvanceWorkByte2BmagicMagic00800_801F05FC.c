#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers. Declared as the same volatile u8 pointer the exact siblings
 * initWorkMotionFieldsBmagicMagic00800_801F0330 and
 * decrementMotionCountdownAndSubtractMotionPairThenAdvanceWorkByte2
 * BmagicMagic00800_801F04DC use: the original re-reads the cell before every
 * store group, and the named symbol lets the assembler fold the constant address
 * into the load (lui $vN,0x1F80 / lw $vN,68($vN)). */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared fixed-point channel pair in main RAM, outside this overlay's image. The
 * scenario lifts rebase these two words as a pair (func_801F9644 commits
 * 0x80149308 = 0x260000 with 0x8014930C = 0x1B8000) and the bmagic sibling
 * seedWorkWordPairAndFlagsThenAdvanceWorkByte2BmagicMagic13103_801F04C0 copies
 * both into the same work-object words 0x34/0x38 that this entry fills. Splat
 * binds both addresses through this target's generated undefined_syms_auto.txt,
 * so no target-map row is needed. */
extern u32 D_80149308[]; /* @source 0x80149308 @kind unknown */
extern u32 D_8014930C[]; /* @source 0x8014930C @kind unknown */

/* Shared battle work pointer in main RAM. The sibling bmagic lifts read the same
 * record through this pointer (seedWorkObjectFromBattleRecordAndAdvanceWorkByte2
 * BmagicMagic04500_801EF4A4 copies its 0x34/0x38 word pair, its 0x3E halfword and
 * its 0x27 byte; decrementMotionCountdownAndSubtractMotionPairThenAdvanceWorkByte2
 * BmagicMagic00800_801F04DC stores 0xFF in its byte 0xB). */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Helper of the main executable at 0x8015D71C. The world lifts declare the same
 * one-argument prototype and pass a constant selector. */
void func_8015D71C(s32 arg0);

/* @source 0x801F05FC
 * @behavior Entry 0 of the five-entry work-state table at 0x801EEC70
 * (0x801F05FC, 0x801F0710, 0x801F07D4, 0x801F0864, 0x801F08C4), which
 * 0x801F0568 dispatches on scratchpad work-object byte 2. Per-entry seed of the
 * tint/motion sequence: it raises bit 0x20 of work byte 0, sets mode byte 0x5C
 * and work byte 0x2B to 1, sets the signed tint triplet at 0x5D/0x5E/0x5F to
 * -0x80 (the half-brightness floor the sibling
 * subtractWorkColorTripletBy3ThenAdvanceWorkByte2WhenChannelIs80 fades away
 * from), copies the shared fixed-point channel pair 0x80149308/0x8014930C into
 * work words 0x34/0x38, copies the shared battle work record's word 0x3C into
 * work word 0x3C, sets the 16.16 motion pair at 0x40/0x44 to 0x18000/0x28000,
 * sets work byte 0x48 to 2, copies the record's byte 0x27 (the elevation byte
 * the sibling 0x801EF4A4 also seeds from that source), invokes the helper at
 * 0x8015D71C with 0x100, re-seeds countdown work byte 9 with 0x20 and finally
 * advances work byte 2 with a byte-width read-modify-write. The original reads
 * the scratchpad pointer cell before each store group and the record pointer
 * cell twice, once for the 0x3C word copy and again for the 0x27 byte. One
 * byte local carries the value written to mode byte 0x5C, work byte 0x2B and the
 * tint triplet: the original materialises 1 once in $v1 and re-uses that
 * register for the -0x80 floor, which is the allocation the separate literal
 * writes do not reproduce. The 0x18-byte frame only keeps $ra across the helper
 * call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedMotionTintAndChannelFieldsThenAdvanceWorkByte2BmagicMagic00800_801F05FC(void) {
  u8 *work;
  u32 *words;
  u8 *tilt;
  u8 *countdown;
  u8 *step;
  s8 *tint;
  s8 value;

  work = g_battle_work;
  work[0] |= 0x20;
  value = 1;
  g_battle_work[0x5C] = value;
  g_battle_work[0x2B] = value;
  tint = (s8 *)g_battle_work;
  value = -0x80;
  tint[0x5D] = value;
  tint = (s8 *)g_battle_work;
  tint[0x5E] = value;
  tint = (s8 *)g_battle_work;
  tint[0x5F] = value;
  words = (u32 *)g_battle_work;
  words[13] = D_80149308[0];
  words[14] = D_8014930C[0];
  words[15] = ((u32 *)D_801EB4E0)[15];
  ((u8 *)words)[0x48] = 2;
  words[16] = 0x18000u;
  words[17] = 0x28000u;
  tilt = g_battle_work;
  tilt[0x27] = D_801EB4E0[0x27];
  func_8015D71C(0x100);
  countdown = g_battle_work;
  countdown[9] = 0x20;
  step = g_battle_work;
  step[2]++;
}
