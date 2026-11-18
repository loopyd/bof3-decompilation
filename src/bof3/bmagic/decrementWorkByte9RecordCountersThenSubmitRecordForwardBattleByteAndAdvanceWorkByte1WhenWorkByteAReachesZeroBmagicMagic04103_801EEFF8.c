#include "bof3/bof3.h"

/* Shared record-submit helper at 0x80196718: submits the battle record it is
 * handed. include/bof3/battle/battle03_internal.h declares
 * `void func_80196718(void* arg0)`, and the sibling
 * decrementWorkByte9ThenSubmitRecordStoreEffectByte9AndAdvanceWorkByte1OnZero
 * BmagicMagic04103_801EEE14 of this overlay calls the same address with that
 * same one-argument shape. That header belongs to another target, so the
 * prototype stays source-local and the link binds the address through the
 * generated undefined_funcs_auto.txt row. */
void func_80196718(void* arg0);

/* Shared battle-byte helper at 0x800AF814, invoked with the zero-extended
 * shared battle byte at 0x80146384 (the original emits lbu $a0,%lo(D_80146384)
 * immediately before the jal, so the argument is one byte wide). No include/
 * header declares it and its role is not proven, so the name stays
 * address-anchored. */
extern void func_800AF814(u8 arg0);

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044. The original re-materialises the
 * cell with its own lui $vN,0x1F80 / lw $vN,0x44($vN) pair for every one of the
 * five accesses, because the counter stores and the pointer-typed call argument
 * may alias the cell itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Base of the 12-byte counter records selected by the work object's unsigned
 * byte 0x9. The original scales that byte by the stride into a byte offset
 * (sll/addu/sll through $at) and folds the +1 and +2 displacements of the
 * second and third counters into this symbol's own address
 * (lui $at,%hi(D_80145BD6 + 0x1) / lbu %lo(D_80145BD6 + 0x1)($at)), so the
 * source keeps one base plus a constant displacement rather than distinct
 * 0x80145BD7/0x80145BD8 rows. This target's exact sibling
 * incrementWorkByte9RecordCountersThenAdvanceWorkByteAAndWorkByte1WhenWorkByte
 * AIs8BmagicMagic04103_801EEEB4 uses the same base, offset local and folded
 * displacements for the byte-9 index.
 * @source 0x80145BD6 @kind table
 */
extern u8 D_80145BD6[];

/* Battle record published at 0x8014638C: this body loads the word once and
 * hands the loaded pointer to func_80196718. Its role is not proven, so the
 * name stays address-anchored and the link binds it through the generated
 * undefined_syms_auto.txt row. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Shared battle byte at 0x80146384: the zero-extended byte this body passes to
 * func_800AF814. It lives outside this overlay, so it keeps its address-
 * qualified name and the link binds it through the generated
 * undefined_syms_auto.txt row. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EEFF8
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044, takes
 *           the work object's unsigned byte at offset 0x9 as the index into the
 *           12-byte counter records at 0x80145BD6 and byte-subtracts 2 from the
 *           selected record's byte 0x0 (lbu / addiu $v0,$v0,-2 / sb). It then
 *           re-reads the cell and the index byte for each of the remaining two
 *           counters and subtracts 1 from the selected record's byte 0x1 and
 *           byte 0x2, the second one addressed through the folded
 *           D_80145BD6 + 0x1 relocation. Finally it re-reads the cell once more,
 *           decrements the work object's byte at offset 0xA (lbu / addiu -1 /
 *           sb), zero-extends that stored byte with andi 0xFF and returns
 *           through the shared epilogue while the value stays non-zero (bnez).
 *           Only when that countdown reaches zero does it submit the battle
 *           record published at 0x8014638C through func_80196718, pass the
 *           shared battle byte at 0x80146384 to func_800AF814 and advance the
 *           work object's state byte at offset 0x1, which steps the overlay work
 *           byte 1 dispatcher at 0x801EF190. The 0x18-byte frame keeps only $ra
 *           across the two calls. Reached through word 4 of the overlay code
 *           pointer run at 0x801EEC04, not by a call, so it takes no arguments,
 *           returns nothing and reads or writes no other state.
 *
 *           Shape: the counter ramp copies the statement order, operand forms and
 *           offset-local spelling of this overlay's exact sibling
 *           incrementWorkByte9RecordCountersThenAdvanceWorkByteAAndWorkByte1When
 *           WorkByteAIs8BmagicMagic04103_801EEEB4 (same index byte 0x9, same
 *           0x80145BD6 base and folded displacements); this function's own
 *           subtractive steps 2/1/1 replace its 2/1/1 additions. The countdown
 *           and submit tail copy the exact cross-target sibling
 *           decrementWorkByteBRecordCountersAndWorkByte9ThenSubmitRecordOnWork
 *           ByteAZeroAndStepWorkByte1BmagicMagic11700_801EEE40, whose second
 *           stage decrements byte 0xA and enters the same two calls on zero;
 *           this body has no non-zero arm, so that arm is dropped rather than
 *           transcribed. The 312 bytes 0x801EEFF8..0x801EF12F match no
 *           byte-identical span: the discriminating index byte 0x9 with the
 *           2/1/1 steps and the absent non-zero arm exist only here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9RecordCountersThenSubmitRecordForwardBattleByteAndAdvanceWorkByte1WhenWorkByteAReachesZeroBmagicMagic04103_801EEFF8(void) {
  u32 offset;

  offset = g_battle_work[9] * 12;
  D_80145BD6[offset] -= 2;
  offset = g_battle_work[9] * 12;
  D_80145BD6[offset + 1] -= 1;
  offset = g_battle_work[9] * 12;
  D_80145BD6[offset + 2] -= 1;
  if (--g_battle_work[0xA] == 0) {
    func_80196718(D_8014638C);
    func_800AF814(D_80146384);
    g_battle_work[1]++;
  }
}
