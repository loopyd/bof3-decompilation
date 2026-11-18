#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map. The cell is re-read for every access group because a byte store through
 * the loaded pointer may alias it, so the original keeps a separate lui/lw pair
 * before the byte-0xA ramp, before the byte-0xB ramp, before the byte-0xA test
 * and before each of the two tail stores. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle target byte at 0x80146384. Its bit 0x40 selects which battler
 * record table a handler of this family walks, the same cell the exact bmagic
 * siblings raiseBattleTargetFlag20ClearBytes9AThenAdvanceWorkByte1BmagicMagic09403_801EEF1C
 * (0x801EEF1C) and
 * forwardWorkByte3SubmitSelectedRecordAndAdvanceWorkByte2WhenGlobalByteBBelow8BmagicMagic09303_801EF264
 * (0x801EF264) read. The still-asm span func_801EF334 of this overlay reads the
 * address too, so Splat's generated undefined_syms_auto.txt binds it and no
 * target-map row is needed. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Local battler record arrays selected by the work record's byte 3: the
 * 0x118-byte-stride entries at 0x801EB630 and the 0x140-byte-stride entries at
 * 0x80145E90, the two bases the still-asm span func_801EF334 and the exact
 * sibling forwardWorkByte3SubmitSelectedRecordAndAdvanceWorkByte2WhenGlobalByteBBelow8BmagicMagic09303_801EF264
 * index with the same strides. The still-asm spans func_801EEC5C and
 * func_801EF334 reference both addresses, so Splat's generated
 * undefined_syms_auto.txt binds them and no target-map row is needed. */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind table */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */

/* Shared status-flag helper at 0x800A9610: raises the flag word it is handed on
 * the battlers selected by the index it is handed. The exact bmagic siblings
 * raiseBattleTargetFlag20ClearBytes9AThenAdvanceWorkByte1BmagicMagic09403_801EEF1C
 * and raiseBattleTargetFlag10WhenWorkByte9SetThenAdvanceWorkByte1BmagicMagic10003_801EED9C
 * call the same address with the same two-argument shape. */
void func_800A9610(u8 arg0, s32 arg1);

/* Shared record-submit helper at 0x80196718: submits the record it is handed. */
void func_80196718(void *arg0);

/* Shared effect helper at 0x8019651C: submits a positional effect for the
 * record it is handed and returns the submitted effect byte. The exact sibling
 * decrementWorkByte9ThenSubmitIndexedRecordStoreEffectByteAResetCountdownAndAdvanceWorkByte2BmagicMagic16903_801EF444
 * passes the same five-argument call from a frame whose stack slot carries the
 * fifth argument. This body forwards the record it just submitted and the
 * offsets -8, -8, -8 and 1, and ignores the returned byte. */
s32 func_8019651C(void *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* @source 0x801EF1B4
 * @behavior Per-frame handler of the work record published at the scratchpad
 *           pointer cell 0x1F800044; it is entry 2 of this overlay's byte-1
 *           handler table at 0x801F0340. It adds four to the record's unsigned
 *           byte 0xA and increments its byte 0xB by one, each at byte width and
 *           through its own reload of the pointer cell. It then re-reads the
 *           cell and tests byte 0xA against 0x10, returning while it differs.
 *           Only on equality does it select a battler record by the record's
 *           own byte 3: with bit 0x40 of the shared battle target byte at
 *           0x80146384 set it walks the 0x118-byte-stride entries based at
 *           0x801EB630 and adds three to the forwarded index, and with that bit
 *           clear it walks the 0x140-byte-stride entries based at 0x80145E90
 *           without the adjustment. The selected record is submitted through
 *           func_80196718, kept in $s0 across that call, then handed to
 *           func_8019651C with the offsets -8, -8, -8 and mode 1, whose fifth
 *           argument lives in this function's 0x20-byte frame at $sp+0x10. That
 *           same arm re-reads the cell and forwards the byte-3 index and the
 *           flag word 0x10 to func_800A9610, which raises that battler status
 *           flag. The function then re-reads the cell twice more, storing 8 into
 *           byte 9 and advancing byte 2 by one. It takes no arguments, returns
 *           nothing and reads or writes no other state.
 *
 *           Statement order, operand forms, offset spelling and declaration
 *           style follow the still-asm span func_801EF334 of this overlay, which
 *           selects the same two record tables through the same 0x80146384 bit
 *           0x40, forwards the same byte-3 index with the same +3 adjustment and
 *           keeps the same separate cell reload after each call, and the exact
 *           sibling forwardWorkByte3SubmitSelectedRecordAndAdvanceWorkByte2When
 *           GlobalByteBBelow8BmagicMagic09303_801EF264 (0x801EF264), which spells
 *           the scaled table expression and the truncated index the same way.
 *           The pointer cell is read once per access group because a byte store
 *           through the loaded pointer may alias it; that grouping and the
 *           named-cell spelling follow those two spans, which reload the same
 *           cell after their calls. The nearest lifted sibling,
 *           incrementWorkByteBAdd4ToWorkByteAThenSetWorkByte9To8AndAdvanceWork
 *           Byte2At10BmagicMagic15103_801F129C (0x801F129C), carries the same
 *           two head ramps in the other order, the same byte-0xA test against
 *           0x10 and the same byte-9 and byte-2 tail, but its arm holds no record
 *           submit and no flag raise and its body is a leaf with no frame, so it
 *           supplies shape only; no lifted source in this tree carries this
 *           span's arm, so the clone lever offered no source. This function's
 *           own immediates 4, 0x10, 8, -8 and 1, its byte order and its
 *           selection by the record's own byte 3 come from this overlay's
 *           original bytes.
 *
 *           Representation note: the first seed spelled the cell with the
 *           constant-address accessor (SPAD_PTR_TABLE) and measured 84/96
 *           instructions with a 0x28-byte frame, because common-subexpression
 *           elimination kept the scratchpad base in $s1 across the calls and
 *           reloaded the cell through it, where the original re-materialises lui
 *           on each group; the named cell measured 96/96.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add4ToWorkByteAAndIncrementWorkByteBThenWhenByteAIs10SubmitSelectedRecordRaiseFlag10SetByte9To8AndAdvanceWorkByte2BmagicMagic09803_801EF1B4(void) {
  u8 *record;

  g_battle_work[0x0A] += 4;
  g_battle_work[0x0B] += 1;
  if (g_battle_work[0x0A] == 0x10) {
    if (D_80146384 & 0x40) {
      record = D_801EB630 + g_battle_work[3] * 0x118u;
      func_80196718(record);
      func_8019651C(record, -8, -8, -8, 1);
      func_800A9610((u8)(g_battle_work[3] + 3), 0x10);
    } else {
      record = D_80145E90 + g_battle_work[3] * 0x140u;
      func_80196718(record);
      func_8019651C(record, -8, -8, -8, 1);
      func_800A9610(g_battle_work[3], 0x10);
    }
    g_battle_work[9] = 8;
    g_battle_work[2] += 1;
  }
}
