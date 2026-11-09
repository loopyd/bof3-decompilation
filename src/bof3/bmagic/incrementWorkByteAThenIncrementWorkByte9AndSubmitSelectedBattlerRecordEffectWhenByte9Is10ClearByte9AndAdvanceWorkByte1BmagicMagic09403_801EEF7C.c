#include "bof3/bof3.h"

/* Shared record-submit helper at 0x80196718: submits the record it is handed.
 * include/bof3/battle/battle03_internal.h declares
 * `void func_80196718(void* arg0)`, and the exact bmagic siblings
 * submitRecordStoreEffectByteBClearWorkByte9ThenAdvanceWorkByte1BmagicMagic01000_801EEC80
 * and decrementWorkByte9ThenSubmitRecordStoreEffectByteASetByte9To8AndAdvance-
 * WorkByte1OnZeroBmagicMagic16203_801EEFA0 call the same address with the same
 * one-argument shape. That header belongs to another target, so the prototype
 * stays source-local. */
void func_80196718(void *arg0);

/* Shared effect helper at 0x8019651C: submits a positional effect for the
 * record it is handed and returns the submitted effect byte.
 * include/bof3/battle/battle03_internal.h declares
 * `s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)` and
 * the exact sibling submitRecordStoreEffectByteBClearWorkByte9ThenAdvanceWork-
 * Byte1BmagicMagic01000_801EEC80 passes the same kind of five-argument call
 * from a 0x20-byte frame whose stack slot carries the fourth offset. This body
 * forwards the record it just submitted and the offsets 6, 4, 8 and 0, and
 * ignores the returned byte. No asm span of this overlay references the address
 * after this lift, so the target map row binds it. */
s32 func_8019651C(void *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map. The cell is re-read for every access group because a byte store through
 * the loaded pointer may alias it, so the original keeps a separate
 * lui/lw pair before the byte-0xA ramp, before the byte-9 ramp and test, and
 * before each of the two tail stores. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle target byte at 0x80146384: the battler index this handler acts
 * on, the same cell the exact sibling
 * raiseBattleTargetFlag20ClearBytes9AThenAdvanceWorkByte1BmagicMagic09403_801EEF1C
 * forwards and the same selector func_800A36F0 of emi/battle/battle15 indexes.
 * The still-asm span func_801EED60 references the address too, so Splat's
 * generated undefined_syms_auto.txt binds it and no target-map row is needed. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Local battler work-record arrays selected by the target index: the party
 * records hold 0x140-byte entries, the enemy records 0x118-byte entries, the
 * split func_800A36F0 of emi/battle/battle15 and selectWorkRecordBase of
 * emi/battle/battle03 apply to the same two bases. The still-asm span
 * func_801EED60 references both addresses, so Splat's generated
 * undefined_syms_auto.txt binds them and no target-map row is needed. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */
extern u8 D_801EB2E8[]; /* @source 0x801EB2E8 @kind unknown */

/* @source 0x801EEF7C
 * @behavior Overlay work-step handler of the record published at the scratchpad
 *           pointer cell 0x1F800044. It ramps the work object's unsigned byte
 *           0xA with a byte-width read-modify-write, re-reads the cell and ramps
 *           byte 9 the same way, then compares the truncated value it just
 *           stored - andi of the stored register, no second load - against 0x10
 *           and returns while it differs. Only on equality does it select the
 *           battler record of the shared target byte at 0x80146384: indices
 *           below three take the 0x140-byte-stride party entries at 0x80145E90,
 *           the rest the 0x118-byte-stride enemy entries at 0x801EB2E8. The
 *           selected record is submitted through func_80196718, kept in $s0
 *           across that call, then handed to func_8019651C with the offsets 6,
 *           4, 8 and 0, whose fifth argument lives in this function's 0x20-byte
 *           frame at $sp+0x10. It then re-reads the work cell, clears byte 9,
 *           re-reads it once more and advances byte 1 by one. The function takes
 *           no arguments, returns nothing and reads or writes no other state.
 *           Statement order, operand forms and declaration style follow the
 *           exact sibling
 *           incrementWorkByte9ThenSetGlobalWorkByteBToFFAndInvokeHelperWhenWorkByte9Is20BmagicMagic09403_801EF0FC
 *           (same pointer cell, same unsigned ramp into a u8 that is stored and
 *           then tested, same single call in the equality arm) and the exact
 *           sibling
 *           submitRecordStoreEffectByteBClearWorkByte9ThenAdvanceWorkByte1BmagicMagic01000_801EEC80
 *           (submit, positional effect, byte-9 clear and byte-1 advance tail).
 *           This function's own immediate 0x10, its record selection by battler
 *           index and its 6/4/8/0 effect offsets come from this target's original
 *           bytes. No extracted payload repeats this span: a byte scan of the
 *           252-byte range over all 394 extracted EMI payloads finds a single
 *           copy, in this target's binary only, so no lifted twin existed to
 *           clone from.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteAThenIncrementWorkByte9AndSubmitSelectedBattlerRecordEffectWhenByte9Is10ClearByte9AndAdvanceWorkByte1BmagicMagic09403_801EEF7C(void) {
  u8 value;
  u8 *record;

  g_battle_work[0xA]++;
  value = g_battle_work[9] + 1;
  g_battle_work[9] = value;
  if (value == 0x10) {
    if (D_80146384 < 3u) {
      record = (u8 *)D_80145E90 + D_80146384 * 0x140u;
    } else {
      record = D_801EB2E8 + D_80146384 * 0x118u;
    }
    func_80196718(record);
    func_8019651C(record, 6, 4, 8, 0);
    g_battle_work[9] = 0;
    g_battle_work[1]++;
  }
}
