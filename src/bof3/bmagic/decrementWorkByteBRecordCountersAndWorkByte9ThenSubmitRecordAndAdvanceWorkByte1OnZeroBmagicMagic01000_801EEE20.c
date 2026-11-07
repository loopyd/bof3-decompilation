#include "bof3/bof3.h"

/* Shared record-submit helper at 0x80196718: submits the record it is handed.
 * include/bof3/battle/battle03_internal.h and
 * include/bof3/battle/battle15_internal.h declare
 * `void func_80196718(void* arg0)`, and the exact bmagic sibling
 * submitRecordStoreEffectByteBClearWorkByte9ThenAdvanceWorkByte1BmagicMagic
 * 01000_801EEC80 calls the same address with the same one-argument shape. */
void func_80196718(void* arg0);

/* Shared helper at 0x800AF814, invoked with the shared battle byte at
 * 0x80146384 that the exact target sibling
 * forwardBattleByteThenRaiseFlag4AndInvokeHelperBmagicMagic01000_801EEF58 also
 * reads. No include/ header declares it and its role is not proven, so the name
 * stays address-anchored and the declaration keeps the single zero-extended
 * byte argument the original encodes (lbu $a0 before the jal). */
void func_800AF814(u8 arg0);

/*
 * Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The cell is read once per access, so the original re-materializes its address
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)) for each access group; the cell is
 * therefore reached through this target-map symbol rather than through a
 * constant address. Splat's generated undefined_syms_auto.txt cannot bind it,
 * so the target map owns the address.
 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/*
 * Counter table selected by the work object's byte 0xB: 12-byte records whose
 * leading bytes are the counters this handler completes. The original expresses
 * the three byte-width decrements as field accesses of the selected record, so
 * the record base carries the 12-byte scale and the +0/+1/+2 field offsets fold
 * into the relocation (the third counter's relocation therefore reads
 * 0x80145BD8). This is the same table and the same access form as the exact
 * sibling incrementWorkByteBRecordCountersAndWorkByte1WhenWorkByte9Is10Bmagic
 * Magic01000_801EED0C, which increments the same three bytes of the same
 * records. The target map owns the base address.
 */
typedef struct BmagicMagic01000CounterRecord {
  u8 counter_00;
  u8 counter_01;
  u8 counter_02;
  u8 pad_03[9];
} BmagicMagic01000CounterRecord;

extern BmagicMagic01000CounterRecord D_80145BD6[]; /* @source 0x80145BD6 @kind table */

/*
 * Battle record published at 0x8014638C: the exact sibling
 * submitRecordStoreEffectByteBClearWorkByte9ThenAdvanceWorkByte1BmagicMagic
 * 01000_801EEC80 loads the same word and hands the record to func_80196718, and
 * this body passes the loaded pointer on the same way. Its role is not proven,
 * so the name stays address-anchored.
 */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * the shared helpers consume. It lives outside this overlay, so it keeps its
 * address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EEE20
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 * per access group and decrements, byte by byte, the counter bytes 0x0/0x1/0x2
 * of the 12-byte record that the work object's unsigned byte 0xB selects in the
 * 0x80145BD6 table: every access re-reads the work object (lbu $v0,0xB($v0)),
 * scales the byte by the 12-byte stride into a byte offset and performs a
 * byte-width read-modify-write (lbu, addiu $v0,$v0,-1, sb). It then re-reads
 * the cell and decrements the work object's countdown byte 9 the same way,
 * testing the stored value against zero (andi $v0,$v0,0xFF, bnez). Only when
 * the countdown reaches zero does it re-read the cell once more and: submit the
 * battle record published at 0x8014638C through func_80196718, pass the shared
 * battle byte at 0x80146384 to func_800AF814, and increment the work object's
 * state byte 1. Each group re-reads the cell because a store through the loaded
 * work pointer may alias it, which is why the original materialises the lui/lw
 * pair five times instead of holding one base. The 0x28-byte frame holds $ra
 * and the outgoing argument area; the counter triple is the byte-wise mirror of
 * the exact sibling incrementWorkByteBRecordCountersAndWorkByte1WhenWorkByte9Is
 * 10BmagicMagic01000_801EED0C, the countdown follows the same sibling's byte 9
 * handling with a zero test, and the submit calls copy the exact sibling
 * submitRecordStoreEffectByteBClearWorkByte9ThenAdvanceWorkByte1BmagicMagic
 * 01000_801EEC80. The function takes no arguments, returns nothing and reads or
 * writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteBRecordCountersAndWorkByte9ThenSubmitRecordAndAdvanceWorkByte1OnZeroBmagicMagic01000_801EEE20(void) {
  D_80145BD6[g_battle_work[0xB]].counter_00--;
  D_80145BD6[g_battle_work[0xB]].counter_01--;
  D_80145BD6[g_battle_work[0xB]].counter_02--;
  if (--g_battle_work[9] == 0) {
    func_80196718(D_8014638C);
    func_800AF814(D_80146384);
    g_battle_work[1]++;
  }
}
