#include "bof3/bof3.h"

/* Shared record-submit helper at 0x80196718: submits the record it is handed.
 * include/bof3/battle/battle03_internal.h declares
 * `void func_80196718(void* arg0)`, and the exact bmagic siblings
 * decrementWorkByte9ThenSubmitRecordStoreEffectByteASetByte9To8AndAdvanceWork
 * Byte1OnZeroBmagicMagic16203_801EEFA0 and
 * submitRecordStoreEffectByteBClearWorkByte9ThenAdvanceWorkByte1BmagicMagic
 * 01000_801EEC80 call the same address with the same one-argument shape. Those
 * headers belong to other targets, so the prototype stays source-local. This
 * target's still-asm span func_801EF0A8 keeps the generated
 * undefined_funcs_auto.txt row alive, so no map row is needed. */
void func_80196718(void* arg0);

/* Shared effect helper at 0x8019651C: submits a positional effect for the
 * record it is handed and returns the submitted effect byte.
 * include/bof3/battle/battle03_internal.h declares
 * `s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)`, and
 * the exact sibling
 * decrementWorkByte9ThenSubmitRecordStoreEffectByteASetByte9To8AndAdvanceWork
 * Byte1OnZeroBmagicMagic16203_801EEFA0 passes the same 0/0/0/1 argument
 * quadruple and stores the low return byte in the work object too. */
extern s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* Battle record published at 0x8014638C: this body reads the pointer once,
 * hands it to both shared calls and keeps it in $s0 across the first one. Its
 * role is not proven, so the name stays address-anchored and the generated
 * undefined_syms_auto.txt row keeps binding it, held alive by this target's
 * still-asm spans func_801EECA4 and func_801EF0A8. */
extern u8* D_8014638C; /* @source 0x8014638C @kind unknown */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map row. The cell is read once per access group because a byte store through
 * the loaded pointer may alias it, so the original re-materialises the base
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)) before the countdown, before the effect
 * byte store, before the mode byte store and before the state byte advance. */
extern u8* g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEEF4
 * @behavior Per-frame step of an overlay work-object handler. It reads the
 *           battle record pointer cell at 0x8014638C into a saved register,
 *           pre-decrements the scratchpad work object's unsigned countdown byte
 *           at offset 0x9 with a byte-width read-modify-write (lbu
 *           $v0,0x9($v1) / addiu $v0,$v0,-1 / sb / andi $v0,$v0,0xFF) and
 *           returns without further work while the truncated value stays
 *           non-zero. Only when the countdown reaches zero does it set the
 *           record's byte 0x5C to one, submit that record through
 *           func_80196718, submit its positional effect through func_8019651C
 *           with the 0/0/0 offsets and mode 1 and store the low return byte,
 *           the submitted effect byte, in the work object's byte 0xA. It then
 *           re-reads the work object and sets its mode byte 0x9 to 8, re-reads
 *           it once more and advances its state byte 0x1 with a byte-width
 *           read-modify-write. The work-object cell is re-read for each of the
 *           three tail stores, so the original keeps three separate cell loads
 *           after the countdown group. The 0x20-byte frame holds $ra, $s0 and
 *           the stack slot of the fifth func_8019651C argument. The function
 *           takes no arguments, returns nothing and reads or writes no other
 *           state.
 *
 *           The body follows the exact sibling
 *           decrementWorkByte9ThenSubmitRecordStoreEffectByteASetByte9To8AndAdva
 *           nceWorkByte1OnZeroBmagicMagic16203_801EEFA0, whose statement order,
 *           operand forms and offset-local spelling it copies. That span
 *           differs from this one only by the record byte 0x5C store and the
 *           constant the store needs, plus the one redundant argument copy the
 *           larger body drops. A comparison of this 172-byte span with every
 *           out/binaries payload found no byte-identical copy, so no lifted
 *           twin existed to clone from.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSetRecordByte5CToOneSubmitRecordStoreEffectByteASetByte9To8AndAdvanceWorkByte1OnZeroBmagicMagic08803_801EEEF4(
    void) {
  u8* record;

  record = D_8014638C;
  if (--g_battle_work[9] == 0) {
    record[0x5C] = 1;
    func_80196718(record);
    g_battle_work[0xA] = func_8019651C(record, 0, 0, 0, 1);
    g_battle_work[9] = 8;
    g_battle_work[1]++;
  }
}
