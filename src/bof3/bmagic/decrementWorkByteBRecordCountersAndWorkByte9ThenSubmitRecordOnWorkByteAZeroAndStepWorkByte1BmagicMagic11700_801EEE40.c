#include "bof3/bof3.h"

/* Shared record-submit helper at 0x80196718: submits the record it is handed.
 * include/bof3/battle/battle03_internal.h declares
 * `void func_80196718(void* arg0)`, and both exact bmagic siblings
 * submitRecordStoreEffectByteBClearWorkByte9SetByteATo2ThenAdvanceWorkByte1
 * BmagicMagic11700_801EEC90 and
 * decrementWorkByteBRecordCountersAndWorkByte9ThenSubmitRecordAndAdvanceWork-
 * Byte1OnZeroBmagicMagic01000_801EEE20 call the same address with the same
 * one-argument shape. */
void func_80196718(void* arg0);

/* Shared helper at 0x800AF814, invoked with the shared battle byte at
 * 0x80146384 that the same bmagic siblings read. No include/ header declares it
 * and its role is not proven, so the name stays address-anchored and the
 * declaration keeps the single zero-extended byte argument the original
 * encodes (lbu $a0 before the jal). */
void func_800AF814(u8 arg0);

/*
 * Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * This overlay's original re-materializes the cell address for every access
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)), so the cell is reached through this
 * target's own map symbol rather than through a constant address. Splat's
 * generated undefined_syms_auto.txt does not bind it, so the target map owns
 * the address.
 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/*
 * Counter table selected by the work object's byte 0xB: 12-byte records whose
 * leading bytes are the counters this handler drains. The original expresses
 * the three decrements as field accesses of the selected record, so the record
 * base carries the byte-0xB scale and the +0/+1/+2 field offsets fold into the
 * relocation (which is why the second counter's relocation is the base symbol
 * plus 0x1 and the third's is the distinct reviewed symbol 0x80145BD8). The
 * same table and access form serve the exact siblings
 * incrementWorkByteBRecordCountersAndWorkByte1WhenWorkByte9Is10BmagicMagic11700
 * _801EED2C and decrementWorkByteBRecordCountersAndWorkByte9ThenSubmitRecordAnd
 * AdvanceWorkByte1OnZeroBmagicMagic01000_801EEE20; Splat keeps binding both
 * addresses through the generated undefined_syms_auto.txt.
 */
typedef struct BmagicMagic11700CounterRecord {
  u8 counter_00;
  u8 counter_01;
  u8 counter_02;
  u8 pad_03[9];
} BmagicMagic11700CounterRecord;

extern BmagicMagic11700CounterRecord D_80145BD6[]; /* @source 0x80145BD6 @kind table */

/* Battle record published at 0x8014638C: this body loads the word and hands the
 * loaded pointer to func_80196718 exactly as the exact bmagic siblings do. Its
 * role is not proven, so the name stays address-anchored. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * the shared helpers consume. It lives outside this overlay, so it keeps its
 * address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EEE40
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 * per access group and decrements, byte by byte, the counter bytes 0x0/0x1/0x2
 * of the 12-byte record that the work object's unsigned byte 0xB selects in the
 * 0x80145BD6 table: every access re-reads the work object (lbu $v0,0xB($v0)),
 * scales the byte by the 12-byte stride into a byte offset and performs a
 * byte-width read-modify-write (lbu, addiu $v0,$v0,-1, sb). It then re-reads
 * the cell and decrements the work object's countdown byte 9 the same way,
 * testing the stored value against zero (andi $v0,$v0,0xFF, bnez). Only when
 * the countdown reaches zero does it enter the second stage: it re-reads the
 * cell and decrements the work object's byte 0xA, testing that byte against
 * zero (andi $v0,$v0,0xFF, beqz). A non-zero byte 0xA steps the state byte 1
 * back by one, while a byte 0xA that reaches zero submits the battle record
 * published at 0x8014638C through func_80196718, passes the shared battle byte
 * at 0x80146384 to func_800AF814 and steps the state byte 1 forward by one. Both
 * arms end in the same byte-width store, which the original shares at one out-
 * of-line block reached by a `j` from the step-back arm, so the forward arm
 * falls through into it. Each group re-reads the cell because a store through
 * the loaded work pointer may alias it, which is why the original materialises
 * the lui/lw pair six times instead of holding one base. The 0x28-byte frame
 * holds $ra and the outgoing argument area. The counter triple and the byte 9
 * countdown are the byte-wise mirror of the exact sibling
 * incrementWorkByteBRecordCountersAndWorkByte1WhenWorkByte9Is10BmagicMagic11700
 * _801EED2C, and the submit calls copy the exact sibling
 * submitRecordStoreEffectByteBClearWorkByte9SetByteATo2ThenAdvanceWorkByte1
 * BmagicMagic11700_801EEC90. The function takes no arguments, returns nothing
 * and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteBRecordCountersAndWorkByte9ThenSubmitRecordOnWorkByteAZeroAndStepWorkByte1BmagicMagic11700_801EEE40(
    void) {
  D_80145BD6[g_battle_work[0xB]].counter_00--;
  D_80145BD6[g_battle_work[0xB]].counter_01--;
  D_80145BD6[g_battle_work[0xB]].counter_02--;
  if (--g_battle_work[9] == 0) {
    if (--g_battle_work[0xA] != 0) {
      g_battle_work[1]--;
    } else {
      func_80196718(D_8014638C);
      func_800AF814(D_80146384);
      g_battle_work[1]++;
    }
  }
}
