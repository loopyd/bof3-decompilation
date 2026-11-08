#include "bof3/bof3.h"

/* Shared record-submit helper at 0x80196718: submits the record it is handed.
 * include/bof3/battle/battle03_internal.h declares
 * `void func_80196718(void* arg0)`, and the exact bmagic siblings
 * submitRecordStoreEffectByteBClearWorkByte9ThenAdvanceWorkByte1BmagicMagic
 * 01000_801EEC80 and
 * decrementWorkByteBRecordCountersAndWorkByte9ThenSubmitRecordAndAdvance-
 * WorkByte1OnZeroBmagicMagic01000_801EEE20 call the same address with the same
 * one-argument shape. That header belongs to another target, so the prototype
 * stays source-local. */
void func_80196718(void* arg0);

/* Shared effect helper at 0x8019651C: submits a positional effect for the
 * record it is handed and returns the submitted effect byte.
 * include/bof3/battle/battle03_internal.h declares
 * `s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)`, and
 * the exact sibling submitRecordStoreEffectByteBClearWorkByte9ThenAdvanceWork-
 * Byte1BmagicMagic01000_801EEC80 passes the same 0/0/0/1 argument quadruple and
 * stores the low return byte in the work object too. */
extern s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* Battle record published at 0x8014638C: this body reads the pointer once,
 * hands it to both shared calls and keeps it in $s0 across the first one. Its
 * role is not proven, so the name stays address-anchored and Splat keeps
 * binding it through the generated undefined_syms_auto.txt row, which the
 * still-asm spans func_801EED38 and func_801EF1A0 of this overlay keep alive. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map row. The cell is read once per access group because a byte store through
 * the loaded pointer may alias it, so the original re-materialises the base
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)) before the countdown, before the effect
 * byte store, before the mode byte store and before the state byte advance. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEFA0
 * @behavior Reads the battle record pointer published at 0x8014638C once per
 * call into a saved register, then decrements the scratchpad work object's
 * unsigned countdown byte at offset 0x9 with a byte-width read-modify-write
 * (lbu $v0,0x9($v1) / addiu $v0,$v0,-1 / sb / andi $v0,$v0,0xFF) and returns
 * without further work while the stored value stays non-zero. Only when the
 * countdown reaches zero does it submit that record through func_80196718 and
 * submit its positional effect through func_8019651C with the 0/0/0 offsets
 * and mode 1, storing the low return byte - the submitted effect byte - in the
 * work object's byte 0xA. It then re-reads the work object and sets its mode
 * byte 0x9 to 8, re-reads it once more and advances its state byte 0x1 with a
 * byte-width read-modify-write. The work-object cell is re-read for each of the
 * three stores, so the original keeps three separate cell loads. The 0x20-byte
 * frame holds $ra, $s0 and the stack slot of the fifth func_8019651C argument.
 * The function takes no arguments, returns nothing and reads or writes no other
 * state. The countdown guard copies the statement order and operand forms of
 * the exact sibling
 * decrementWorkByte9ThenIncrementWorkByte1WhenZeroBmagicMagic00903_801EF01C,
 * and the submit/store tail copies the exact sibling
 * submitRecordStoreEffectByteBClearWorkByte9SetByteATo2ThenAdvanceWorkByte1
 * BmagicMagic11700_801EEC90, with this function's own effect byte offset 0xA,
 * mode constant 8 and byte 0xA instead of 0xB. The 0xA8-byte range is also
 * byte-identical to six still-asm spans of this family (targets magic062 at
 * 0x801EEEC0, magic070/magic071/magic072 at 0x801EEFA4, magic082 at 0x801EF0AC
 * and magic168 at 0x801EEFA0), so no lifted twin existed to clone from.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSubmitRecordStoreEffectByteASetByte9To8AndAdvanceWorkByte1OnZeroBmagicMagic16203_801EEFA0(
    void) {
  u8* record;

  record = D_8014638C;
  if (--g_battle_work[9] == 0) {
    func_80196718(record);
    g_battle_work[0xA] = func_8019651C(record, 0, 0, 0, 1);
    g_battle_work[9] = 8;
    g_battle_work[1]++;
  }
}
