#include "bof3/bof3.h"

/* 0x118-byte local panel record row: this function reads only the kind byte that
 * comes first. The exact bmagic sibling
 * testBattleTargetKindFlagBmagicMagic06000_801EF118 declares the same row for
 * the same 0x118-byte stride, and the near-identical body at 0x801EEEFC in the
 * magic044 #3 payload binds the same shape for the same indexed byte read. */
typedef struct BmagicMagic17303LocalPanelRow {
  u8 kind;
  u8 unk_01[0x118 - 1];
} BmagicMagic17303LocalPanelRow;

/* Local panel record array in main RAM, outside this overlay image. Splat binds
 * the address through this target's generated undefined_syms_auto.txt row, so a
 * D_ name needs no target-map row of its own. */
extern BmagicMagic17303LocalPanelRow D_801EB6AC[]; /* @source 0x801EB6AC @kind table */

/* Row of the pair table at 0x801F09B0 in this overlay's trailing read-only
 * block: 0x80 bytes, the pitch the original forms with a single sll by seven of
 * work byte 0x4, carrying four named halfwords at the front. The exact
 * same-target sibling
 * insertEmptyRecordAtWorkByteBAndShiftRowRecordsUpBmagicMagic17303_801F03A0
 * indexes the same table in 4-byte records, 32 per row, so the two files bind
 * the view each one reads. */
typedef struct BmagicMagic17303PairRow {
  u16 unk_00;
  u16 unk_02;
  u16 unk_04;
  u16 unk_06;
  u8 unk_08[0x80 - 8];
} BmagicMagic17303PairRow;

/* Pair table in this overlay's trailing read-only block. Splat binds the address
 * through the generated undefined_syms_auto.txt row that the remaining asm spans
 * func_801EF4EC, func_801EFA34 and func_801F02B0 still reference. */
extern BmagicMagic17303PairRow D_801F09B0[]; /* @source 0x801F09B0 @kind data */

/* Shared battle record pointer in main RAM, outside this overlay image; the
 * exact bmagic siblings seedWorkMotionFromBattlerRecordAndAdvanceWorkByte2
 * BmagicMagic05403_801EF238 and seedWorkObjectFromBattlerRecordWhenBattlerReady
 * AndStateByteNotSixElseFinishWorkByte2BmagicMagic01200_801EF4B8 read the same
 * pointer and the same words 0x08, 0x34, 0x38 and 0x3C through it. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Record pointer at 0x8014638C in main RAM. This lift reads only words 0x34 and
 * 0x38 of the record it points to; its role is not proven, so the name stays
 * address-anchored, exactly as the exact siblings
 * copyRecordTripletMinus4000Plus4000000SetBytes9BTo10AndAToB0ThenAdvanceWorkByte2
 * BmagicMagic07903_801EEE64 and copyWorkWordPairAndHalfwordClearBytes9AThenAdvance
 * WorkByte2BmagicMagic10303_801EEF94 declare it. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Acting battler byte in main RAM. A value below 3 addresses the party block of
 * the 0x118-byte-stride local panel records; the 3 or more case is that array
 * index minus three, exactly as the exact bmagic sibling
 * testBattleTargetKindFlagBmagicMagic06000_801EF118 indexes the same array from
 * the battle target byte 0x80146384. */
extern u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * with its own lui 0x1F80 plus lw 0x44 pair at every read, because a store
 * through the loaded pointer may alias the cell, so the cell is named rather
 * than written as a bare scratchpad constant. The exact same-target sibling
 * decrementWorkByte9By4ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZero
 * BmagicMagic17303_801F0700 declares the same cell the same way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Work-object setup helper reached here with the work object as its only
 * argument, as in the exact bmagic sibling seedWorkObjectFromBattlerRecord
 * WhenBattlerReadyAndStateByteNotSixElseFinishWorkByte2BmagicMagic01200_801EF4B8,
 * the two-argument heading helper of that sibling, and the no-argument helper the
 * same-target sibling seedWorkHeaderClearTintGroupAndRequestResourceThenAdvance
 * WorkByte2BmagicMagic17303_801F0540 calls. All three are outside this overlay
 * image, keep their raw names and are bound by this target's generated
 * undefined_funcs_auto.txt rows. */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */
extern void func_800AF764(void); /* @source 0x800AF764 */

/* @source 0x801EEEEC
 * @behavior Entry 2 of the ten-entry code-pointer run based at 0x801F0988,
 *           which this overlay's step dispatcher
 *           dispatchStateByte1Table0988BmagicMagic17303_801EEDF0 indexes with the
 *           scratchpad work-object byte 0x1. It takes no arguments and returns
 *           nothing, and its final increment of work byte 0x2 hands the next
 *           frame to the byte-2 handler run based at 0x801F09A0.
 *           The record pointer at 0x8014638C is read once into a saved register
 *           before the byte copy, because the store through the work pointer may
 *           alias it; the shared battle record pointer at 0x801EB4E0 and the
 *           work pointer cell at 0x1F800044 are read in the same prologue, and
 *           record byte 0x08 is copied into work byte 0x08.
 *           The work motion triple at 0xC, 0x10 and 0x14 is then selected from
 *           the acting battler byte 0x80146374. A battler index below 3 takes
 *           word 0xC = 0x4000, word 0x10 = 0 and word 0x14 = 0xC00000. A battler
 *           index of 3 or more takes the kind byte of the 0x118-byte-stride local
 *           panel record at 0x801EB6AC indexed by that index minus three: kind
 *           0x61 takes 0x38000 with a zero word 0x14, kind 0x6C takes 0x20000
 *           when bit 0x2 of work byte 0x08 is set and 0x30000 otherwise, with
 *           word 0x14 = 0x800000, and every other kind takes 0x10000 and
 *           0x800000. Every arm clears word 0x10 and reads the pointer cell for
 *           its own work base.
 *           The dispatch shape is evidence for a two-case switch, not for an
 *           if chain: the original tests kind 0x61, then kind 0x6C, then takes a
 *           plain j to the default block, which sits last after both case blocks
 *           instead of falling through from the tests, and emits no range test
 *           because the case set is that small. The kind 0x6C arm tests bit 0x2
 *           of work byte 0x08 with bnez, materialises 0x20000 in that branch
 *           delay slot and 0x30000 on the fall-through, so it is an if/else on
 *           that bit that the original if-converts into one select: a ternary
 *           spelling of the same select measured an inverted beqz and moved the
 *           value into another register on this span.
 *           It then passes the work object read again from the cell to the
 *           shared helper func_801DD350, and adds the words at 0x34, 0x38 and
 *           0x3C of the battle record into the same three words of the work
 *           object through the motion triple it just stored.
 *           The heading is func_8017A138 of the differences of words 0x34 and
 *           0x38 between the record the saved pointer addresses and the updated
 *           work object; the answer replaces work word 0xC, work word 0x10 is
 *           cleared again and the shared helper func_800AF764 is called with the
 *           clearing store in its delay slot.
 *           Last it publishes the work halfword pair at 0x2E and 0x30 into the
 *           two leading halfword pairs of row work[0x4] of the 0x80-byte-row
 *           pair table at 0x801F09B0, clears work byte 0xB, sets work byte 0x9
 *           to 0x10 and work byte 0xA to 1, and advances work byte 0x2. The four
 *           table stores and the byte 0xB store share one work base while the
 *           three later byte stores each re-read the pointer cell, which is why
 *           only those three carry a fresh lui plus lw pair. The 0x18-byte frame
 *           keeps the return address at 0x14 and the saved record pointer at
 *           0x10.
 *
 *           Shape, statement order and declaration style come from the
 *           near-identical body at 0x801EEEFC in the magic044 #3 payload, whose
 *           0x23C-byte span is this one with the pair table rebased to
 *           0x801F0A38 and a different battler-kind set. A word comparison of
 *           the two spans shows the tail equal except the four %lo
 *           displacements of the table base, the prologue equal except the
 *           absolute delay-slot constants of the party arm, and the kind
 *           dispatch structurally different, so that body is shape evidence and
 *           not a clone: a scan of every payload under out/binaries finds no
 *           copy of this 0x25C-byte span. This span's own literals, case values,
 *           ternary and switch topology are written from these bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkMotionFromBattlerClassThenSetHeadingToward8014638CPublishHalfwordPairAndAdvanceWorkByte2BmagicMagic17303_801EEEEC(void) {
  u8 *work;
  u8 *headingWork;
  u8 *pairWork;
  u8 *partyWork;
  u8 *kind61Work;
  u8 *kind6cWork;
  u8 *defaultWork;
  u8 *source;
  u8 *record;
  s32 heading;

  source = D_8014638C;
  g_battle_work[8] = D_801EB4E0[8];
  if (D_80146374 < 3) {
    partyWork = g_battle_work;
    *(u32 *)(partyWork + 0xC) = 0x4000;
    *(u32 *)(partyWork + 0x10) = 0;
    *(u32 *)(partyWork + 0x14) = 0xC00000;
  } else {
    switch (D_801EB6AC[D_80146374 - 3].kind) {
    case 0x61:
      kind61Work = g_battle_work;
      *(u32 *)(kind61Work + 0xC) = 0x38000;
      *(u32 *)(kind61Work + 0x10) = 0;
      *(u32 *)(kind61Work + 0x14) = 0;
      break;
    case 0x6C:
      kind6cWork = g_battle_work;
      if (kind6cWork[8] & 2) {
        *(u32 *)(kind6cWork + 0xC) = 0x20000;
      } else {
        *(u32 *)(kind6cWork + 0xC) = 0x30000;
      }
      *(u32 *)(kind6cWork + 0x10) = 0;
      *(u32 *)(kind6cWork + 0x14) = 0x800000;
      break;
    default:
      defaultWork = g_battle_work;
      *(u32 *)(defaultWork + 0xC) = 0x10000;
      *(u32 *)(defaultWork + 0x10) = 0;
      *(u32 *)(defaultWork + 0x14) = 0x800000;
      break;
    }
  }

  func_801DD350(g_battle_work);
  record = D_801EB4E0;
  work = g_battle_work;
  *(u32 *)(work + 0x34) = *(u32 *)(record + 0x34) + *(u32 *)(work + 0xC);
  *(u32 *)(work + 0x38) = *(u32 *)(record + 0x38) + *(u32 *)(work + 0x10);
  *(u32 *)(work + 0x3C) = *(u32 *)(record + 0x3C) + *(u32 *)(work + 0x14);
  heading = func_8017A138(*(s32 *)(source + 0x34) - *(s32 *)(work + 0x34),
                          *(s32 *)(source + 0x38) - *(s32 *)(work + 0x38));
  headingWork = g_battle_work;
  *(u32 *)(headingWork + 0xC) = heading;
  *(u32 *)(headingWork + 0x10) = 0;
  func_800AF764();
  pairWork = g_battle_work;
  D_801F09B0[pairWork[4]].unk_00 = *(u16 *)(pairWork + 0x2E);
  D_801F09B0[pairWork[4]].unk_02 = *(u16 *)(pairWork + 0x30);
  D_801F09B0[pairWork[4]].unk_04 = *(u16 *)(pairWork + 0x2E);
  D_801F09B0[pairWork[4]].unk_06 = *(u16 *)(pairWork + 0x30);
  pairWork[0xB] = 0;
  g_battle_work[9] = 0x10;
  g_battle_work[0xA] = 1;
  g_battle_work[2]++;
}
