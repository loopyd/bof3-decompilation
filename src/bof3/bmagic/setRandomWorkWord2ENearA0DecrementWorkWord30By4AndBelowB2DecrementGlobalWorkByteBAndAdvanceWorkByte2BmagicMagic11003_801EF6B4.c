#include "bof3/bof3.h"

extern int rand(void);

/* Scratchpad work-object pointer cell, bound by this target's own map row
 * g_battle_work = 0x1F800044 and spelled that way by this target's other exact
 * lifts. The body reads the cell after the rand() call and addresses the two
 * work halfwords through that one loaded base, then reads the cell a second
 * time in the guarded tail, so the reload is a fresh pointer load and not one
 * base kept live across the call. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the remaining assembly spans func_801EEC94, func_801EF740 and func_801EF8F8
 * load it, so the name needs no map row, and the access here is byte-addressed,
 * so the local declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF6B4
 * @behavior Entry 6 of the fourteen-word byte-1 handler table based at
 *           0x801F046C, which dispatchStateByte1Table046cBmagicMagic11003_801EF3F8
 *           indexes with work record byte 1 and calls indirectly; the word at
 *           0x801F0484 is the only word in the payload that holds this address,
 *           so the entry has no jal caller. Calls rand() and leaves the call
 *           delay slot as a nop, then reads the scratchpad work pointer cell at
 *           0x1F800044 and stores the low two bits of the answer plus 0x9F into
 *           the unsigned halfword at 0x2E (andi 3 / addiu 0x9F / sh). Reads the
 *           unsigned halfword at 0x30 into a sixteen-bit local, subtracts four
 *           (lhu / addiu -4), stores it back at halfword width, and tests the
 *           stored value sign-extended from sixteen bits against 0xB2 (sll 16 /
 *           sra 16 / slti), so the guarded tail runs only while that halfword
 *           stays below 0xB2 under a signed compare. In the tail it reads the
 *           shared battle work record pointer at 0x801EB4E0, decrements that
 *           record's byte 0xB by one at byte width (lbu / addiu -1 / sb), reads
 *           the scratchpad cell again and increments work record byte 2, which
 *           advances the work-byte-2 dispatcher to its next handler step. The
 *           0x18-byte frame keeps only $ra at 0x10 across the call, so the
 *           prologue is addiu $sp,-0x18 / sw $ra,0x10($sp). Takes no arguments,
 *           returns nothing and reads or writes no other state. The 140-byte
 *           span 0x801EF6B4..0x801EF73F carries no byte-identical twin: a
 *           word-wise scan of all 396 extracted payloads with every jal target
 *           masked found only this span's own word, so the clone lever is
 *           declined and the body is seeded from the nearest exact shape
 *           sibling, setRandomWorkWord2ENearA0DecrementWorkWord30By2
 *           AdvanceWorkWord44By2000AndAdvanceWorkByte2Past17FFFBmagicMagic11003_801EF648,
 *           whose rand call with a nop delay slot, 0x9F halfword store at 0x2E,
 *           halfword read-modify-write at 0x30 and closure over work byte 2 are
 *           kept while every literal, offset, width and the guard here come
 *           from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setRandomWorkWord2ENearA0DecrementWorkWord30By4AndBelowB2DecrementGlobalWorkByteBAndAdvanceWorkByte2BmagicMagic11003_801EF6B4(
    void) {
  s32 roll;
  u8 *work;
  s16 value;

  roll = rand();
  work = g_battle_work;
  *(u16 *)(work + 0x2E) = (roll & 3) + 0x9F;
  value = *(u16 *)(work + 0x30) - 4;
  *(u16 *)(work + 0x30) = value;
  if (value < 0xB2) {
    D_801EB4E0[0x0B]--;
    g_battle_work[0x02]++;
  }
}
