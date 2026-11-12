#include "bof3/bof3.h"

/* Scratchpad cursor cell every overlay step reads its work slot from, and the
 * scratchpad halfword that publishes the slot index being activated. */
extern u8 *D_1F800044;
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Shared 0x98-byte work-record table of the main-executable data segment: the
 * same stride, flag byte 0 and 0x38 word the exact same-target lift
 * convergeWorkRecordPairYOverCountdownThenAdvanceWorkByte1ScenarioScena1300_801F7958
 * moves, and the flags byte the exact same-target lift
 * clearTwoSelectedWorkRecordFlagsThenSubmitBothAndResetWorkAreaScenarioScena1300_801F7C24
 * clears through the same record view. Byte 0x5C is the record byte the scena03
 * sibling of this body clears; this handler stores 1 into it. Fields this body
 * never addresses stay unnamed padding. */
typedef struct ScenarioScena1300WorkRecord {
  u8 flags_00;
  u8 pad_01[0x37];
  u32 word_38;
  u8 pad_3C[0x20];
  u8 byte_5C;
  u8 pad_5D[0x3B];
} ScenarioScena1300WorkRecord; /* 0x98 bytes */

/* The target's generated undefined_syms_auto.txt binds the base address. */
extern ScenarioScena1300WorkRecord D_80146888[]; /* @source 0x80146888 @kind data */

/* In-image activation entry that follows this overlay's twelve-word work-object
 * handler table D_801FBEF0. Both activations of this handler hand the same
 * entry to the shared activator; its role beyond being the activation template
 * is unknown, so it stays address-anchored. */
extern u8 D_801FBF20[]; /* @source 0x801FBF20 @kind unknown */

/* Shared main-executable services, all outside this overlay's image and all
 * bound by the target's generated undefined_funcs_auto.txt.
 *
 * func_8015CA48 is the slot allocator the scenario lifts call twice in a row:
 * it hands out identifiers in 0x00..0xFF and returns 0xFF when no slot is left,
 * which is the value this body tests twice.
 *
 * func_801A4D54 takes one in-image activation entry.
 *
 * func_8019651C takes the selected record plus four scalar arguments, the last
 * one on the stack. The roles of those scalars are not evidenced, so the
 * prototype keeps positional names. */
u8 func_8015CA48(void);
void func_801A4D54(void *arg0);
s32 func_8019651C(void *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* @source 0x801F763C
 * @behavior Work-byte-1 handler of this overlay, entry 8 (the word at
 *           0x801FBF10) of the twelve-word work-object handler table D_801FBEF0
 *           that dispatchWorkByte1HandlerScenarioScena1300_801F6EA4 selects
 *           with byte 1 of the scratchpad work object published at 0x1F800044;
 *           that one data word is this body's only reference in the payload.
 *           It takes no arguments and returns nothing.
 *
 *           It asks the shared slot allocator func_8015CA48 for a free
 *           identifier and stores it at cursor byte 3, returning when that
 *           identifier is 0xFF. Otherwise it sets the flags byte of the record
 *           the identifier scales, asks func_8015CA48 for a second identifier at
 *           cursor byte 4, and when that one is 0xFF clears the first record
 *           flags byte again and returns. With two identifiers it sets the
 *           second record flags byte, publishes each identifier as the halfword
 *           at 0x1F800000 and hands both activations the local entry at
 *           0x801FBF20 through func_801A4D54, republishing the work object at
 *           0x1F800044. It then opens the 0x38 word of the two records by
 *           -0x10000 for the record the first identifier scales and by +0x10000
 *           for the second, sets bit 0x20 of both flags bytes, stores 1 into
 *           byte 0x5C of both records (the second identifier's first), submits
 *           both through func_8019651C with the 0xF mask and mode 1, and
 *           finally arms cursor byte 9 to 0x20 and switches cursor byte 1 to 1.
 *
 *           Shape facts from the original bytes: every work-byte and record-byte
 *           access after a store goes back through the published pointer cell,
 *           so the cell is re-read once per access group and each record
 *           subscript re-reads its identifier; the cursor stays in $s2 from the
 *           activation pair to the two submits; the identifiers are read back
 *           through the cell rather than reused from the allocator's return; the
 *           0xFF constant is hoisted to $s0 for both range tests and the stored
 *           flag value 1 to $s3 for all three of its stores; the 0x1F800000 and
 *           0x80146888 bases are each materialised once into a saved register;
 *           and each record access re-forms its own `lui %hi` / `addu $at,$at,
 *           scaled` / `%lo($at)` pair, which is what the strided record view
 *           emits and what a named scalar offset local measured away. The
 *           0x10000 magnitude is written as a subtraction of an unsigned
 *           0x10000, which the owning compiler folds into `lui $a0,0xFFFF` plus
 *           `addu` for the first record and keeps as `lui $a0,1` plus `addu`
 *           for the second.
 *
 *           Statement order, operand forms and declaration style come from the
 *           nearer sibling
 *           claimTwoWorkRecordsActivateBothWithScriptSpreadAccumulatorsAndSubmitScenarioScena0300_801F754C
 *           (0x801F754C), whose instruction run is the same body: the two spans
 *           differ only in that sibling's extra 0x801468BC word pair for both
 *           records, in its 0x1000 magnitudes against the 0x10000 this span
 *           writes, in its 0 store into the record byte 0x5C against the 1 here,
 *           in its cursor byte 9 value 4 against 0x20 here, and in the two
 *           spans' own activation-entry and internal branch addresses. That span
 *           is shape evidence only: one payload scan over out/binaries found
 *           these 796 bytes, and the 0x40-byte window holding the two distinct
 *           0x10000 immediates, at offset 0xA3C of this payload alone, so no
 *           byte-identical twin exists and this span was not cloned.
 * @status partial
 * @match 96.00
 * @residual This build measures 192 of 199 instructions (96.00%) and 796 ->
 *           800 bytes
 *           against the 199-instruction original. The extra word is a `nop` in
 *           the load-delay slot after `lbu $v1,0x4($v0)`, where the original
 *           schedules `addiu $a1,$zero,0xF` there, and the two `beq`/`j`
 *           instructions that name the shared return label shift by that one
 *           word. Every other instruction matches, including the 0x38-byte
 *           frame with its five saves, all nine scaled-index chains and every
 *           record address. The frame needs the unreferenced 8-byte local
 *           below: without it this build reserves 0x30 where the original
 *           reserves 0x38. Measured and rejected this lane: a flat byte-array
 *           spelling with a named `u32 offset` scalar (151/200, first
 *           difference the third scaled chain, which ends `sll $v1,$v0,3`
 *           where the original ends `sll $v0,$v0,3`), the same with a `s32`
 *           offset, with a `152u` multiplier, with a `s32` accumulator and
 *           with `accum += 0x10000u` (151/200 each), a fully inline spelling
 *           with no named offset (99/199, 752 bytes), a named `accum` word
 *           local over the record field (184/200), a `u8 value` flag local
 *           (192/200, no change) and four declaration orders of the
 *           unreferenced frame local (192/200 each). The remaining miss is the
 *           scheduler's placement of one argument constant, the same class the
 *           scena03 and world02 siblings of this body retain, so the clean-C
 *           ladder is not claimed exhausted.
 */
void claimTwoWorkRecordsThenActivateBothWithSharedEntryOpenWord38By10000SetFlag20AndArmByte9ScenarioScena1300_801F763C(
    void) {
  /* Unreferenced frame reservation. The prologue reserves 0x38 bytes while this
   * body's only stack references are the five saves and the two 0x10 outgoing
   * argument stores, so 8 of those bytes are dead local space in the original
   * translation unit. Declaring one 8-byte local restores that reservation; the
   * name and type are a reconstruction choice constrained only by the slot
   * size. */
  u32 unusedFrameSlot[2];
  u8 *cursor;
  u8 *pair;
  u16 *slot;

  (void)unusedFrameSlot;
  D_1F800044[3] = func_8015CA48();
  if (D_1F800044[3] == 0xFF) {
    return;
  }

  D_80146888[D_1F800044[3]].flags_00 = 1;
  D_1F800044[4] = func_8015CA48();
  if (D_1F800044[4] == 0xFF) {
    D_80146888[D_1F800044[3]].flags_00 = 0;
    return;
  }

  D_80146888[D_1F800044[4]].flags_00 = 1;

  cursor = D_1F800044;
  slot = &D_1F800000;

  *slot = cursor[3];
  func_801A4D54(&D_801FBF20);
  *slot = cursor[4];
  func_801A4D54(&D_801FBF20);
  D_1F800044 = cursor;

  D_80146888[cursor[3]].word_38 -= 0x10000u;
  D_80146888[cursor[4]].word_38 += 0x10000;

  D_80146888[cursor[3]].flags_00 |= 0x20;
  D_80146888[D_1F800044[4]].flags_00 |= 0x20;

  pair = D_1F800044;
  D_80146888[pair[4]].byte_5C = 1;
  D_80146888[pair[3]].byte_5C = 1;

  func_8019651C(&D_80146888[cursor[3]], 0xF, 0, 0, 1);
  func_8019651C(&D_80146888[cursor[4]], 0, 0, 0xF, 1);

  D_1F800044[9] = 0x20;
  D_1F800044[1] = 1;
}
