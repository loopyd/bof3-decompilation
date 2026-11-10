#include "bof3/bof3.h"

/* Shared record-submit helper at 0x80196718: submits the battle record it is
 * handed. include/bof3/battle/battle03_internal.h declares
 * `void func_80196718(void* arg0)`, and the sibling
 * decrementWorkByteBRecordCountersAndWorkByte9ThenSubmitRecordAndAdvance
 * WorkByte1OnZeroBmagicMagic01000_801EEE20 calls the same address with that
 * same one-argument shape. That header belongs to another target, so the
 * prototype stays source-local and the link binds the address through the
 * generated undefined_funcs_auto.txt row, which the still-asm span
 * func_801EEFF8 of this overlay keeps alive. */
void func_80196718(void* arg0);

/* Shared effect helper at 0x8019651C: submits a positional effect for the
 * record it is handed and returns the submitted effect byte.
 * include/bof3/battle/battle03_internal.h declares
 * `s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)`;
 * this body passes the 0/0/0 offsets plus mode 1 in the fifth argument slot
 * and stores the low return byte, the operand form the sibling
 * decrementWorkByte9ThenSubmitRecordStoreEffectByteASetByte9To8AndAdvance
 * WorkByte1OnZeroBmagicMagic16203_801EEFA0 uses. */
extern s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* Battle record published at 0x8014638C: a pointer cell this body reads once
 * per shared call through one held cell address, so the original computes the
 * cell address into $s0 (lui/addiu) and re-loads the word with a zero
 * displacement for each call because a call may store through it. Its role is
 * not proven, so the name stays address-anchored and the link binds it through
 * the generated undefined_syms_auto.txt row, which the still-asm span
 * func_801EEFF8 of this overlay keeps alive. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map row. The original re-materialises the base with its own
 * lui $v1,0x1F80 / lw $v1,0x44($v1) pair for the countdown, for the effect
 * byte store and for the state byte advance, because the byte stores through
 * the loaded pointer may alias the cell itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEE14
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 before
 *           the frame is set up, decrements the work object's unsigned byte at
 *           offset 0x9 at byte width (lbu $v0,0x9($v1) / addiu $v0,$v0,-1 /
 *           sb / andi $v0,$v0,0xFF) and returns without further work while the
 *           stored value stays non-zero. Only when the countdown reaches zero
 *           does it take the battle record pointer published at 0x8014638C
 *           into $s0 (lui/addiu of the cell, then lw $a0,0($s0)) and submit
 *           that record through func_80196718; it then reloads the same cell
 *           word and calls func_8019651C with the 0/0/0 offsets and mode 1 in
 *           the fifth argument slot at 0x10($sp), storing the low return byte
 *           - the submitted effect byte - back into the work object's byte
 *           0x9. It re-reads the work-object cell once more and advances the
 *           work object's state byte at offset 0x1 at byte width (lbu / addiu
 *           $v0,$v0,1 / sb), which is the handler index the overlay step
 *           dispatcher at 0x801EF190 advances through. The 0x20-byte frame
 *           holds $ra at 0x1C, $s0 at 0x18 and the fifth argument slot at
 *           0x10; the function takes no arguments, returns nothing and reads
 *           or writes no other state. The overlay's leading pointer run at
 *           0x801EEC04 reaches this address through its word 1, so no jal in
 *           the image calls it.
 *
 *           Shape: the countdown guard and the submit/store/advance tail copy
 *           the statement order, operand forms and declaration style of the
 *           sibling
 *           decrementWorkByte9ThenSubmitRecordStoreEffectByteASetByte9To8-
 *           AndAdvanceWorkByte1OnZeroBmagicMagic16203_801EEFA0, which was
 *           written exact; this body differs from it by the extra work byte
 *           0x9 = 8 store that one carries, by storing the effect byte at byte
 *           0x9 instead of 0xA, and by re-reading the record cell inside the
 *           guard. A scan of every extracted payload under out/binaries found
 *           no copy of the 0xA0 bytes 0x801EEE14..0x801EEEB3 in any other
 *           image, so no clone applied. The held cell address is the shape the
 *           bytes require: two direct uses of the cell name fold the address
 *           into each load (lui $a0,%hi / lw $a0,%lo($a0), measured at 31/40
 *           instructions and 152 bytes), while one cell pointer dereferenced
 *           per call reproduces the original 40 instructions in 160 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSubmitRecordStoreEffectByte9AndAdvanceWorkByte1OnZeroBmagicMagic04103_801EEE14(
    void) {
  u8** cell;

  if (--g_battle_work[9] == 0) {
    cell = &D_8014638C;
    func_80196718(*cell);
    g_battle_work[9] = func_8019651C(*cell, 0, 0, 0, 1);
    g_battle_work[1]++;
  }
}
