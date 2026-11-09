#include "bof3/bof3.h"

/* 0x118-byte local panel record row: this function reads only the kind byte
 * that comes first. The exact bmagic sibling
 * testBattleTargetKindFlagBmagicMagic06000_801EF118 declares the same row for
 * the same 0x118-byte stride, and the battle target reads it through its own
 * private BattleLocalPanelRecord, so this overlay binds the shape it reads
 * instead of including another target header. */
typedef struct BmagicMagic04403LocalPanelRow {
  u8 kind;
  u8 unk_01[0x118 - 1];
} BmagicMagic04403LocalPanelRow;

/* Local panel record array in main RAM, outside this overlay image. Splat binds
 * the address through this target generated undefined_syms_auto.txt, which the
 * remaining asm spans of this overlay still reference; a D_ name needs no map
 * row of its own. */
extern BmagicMagic04403LocalPanelRow D_801EB6AC[]; /* @source 0x801EB6AC @kind table */

/* Row of this overlay pair table at 0x801F0A38: 0x80 bytes, the pitch the
 * original forms with a single sll by seven of work byte 0x4, carrying four
 * named halfwords at the front. The exact same-target sibling
 * insertEmptyRecordAtWorkByteBAndShiftRowRecordsUpBmagicMagic04403_801F0390
 * indexes the same table in 4-byte records, 32 per row. */
typedef struct BmagicMagic04403PairRow {
  u16 unk_00;
  u16 unk_02;
  u16 unk_04;
  u16 unk_06;
  u8 unk_08[0x80 - 8];
} BmagicMagic04403PairRow;

/* Pair table in this overlay trailing read-only block. Splat binds the address
 * through the generated undefined_syms_auto.txt row that the remaining asm
 * spans func_801EF4E0, func_801EFA4C and func_801F02A0 still reference. */
extern BmagicMagic04403PairRow D_801F0A38[]; /* @source 0x801F0A38 @kind data */

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
 * than written as a bare scratchpad constant. The exact same-target siblings
 * dispatchWorkByte2HandlerThenDrawByWorkByte4BmagicMagic04403_801EEE44 and
 * queueSlotSetByte1PublishRecordPointerCopyWorkWordTripletEveryFourthTickWhile
 * ByteABelow16ThenAdvanceWorkByte2At16BmagicMagic04403_801EF320 declare the same
 * cell the same way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Work-object setup helper reached here with the work object as its only
 * argument, as in the exact bmagic sibling seedWorkObjectFromBattlerRecord
 * WhenBattlerReadyAndStateByteNotSixElseFinishWorkByte2BmagicMagic01200_801EF4B8,
 * the two-argument heading helper of that sibling and of func_801EFA4C, and the
 * no-argument helper the same-target sibling queueSlotSetByte1PublishRecord
 * PointerCopyWorkWordTripletEveryFourthTickWhileByteABelow16ThenAdvanceWork
 * Byte2At16BmagicMagic04403_801EF320 calls. All three are outside this overlay
 * image and keep their raw names. */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */
extern void func_800AF764(void); /* @source 0x800AF764 */

/* @source 0x801EEEFC
 * @behavior Entry 0 of the three-entry work-byte-2 handler table at 0x801F0A1C
 *           that this overlay byte 2 dispatcher
 *           dispatchWorkByte2HandlerThenDrawByWorkByte4BmagicMagic04403_801EEE44
 *           indexes with the scratchpad work-object byte 0x2, so that byte holds
 *           0 while this body runs and its final increment hands the next frame
 *           to the second entry at 0x801EF138. It takes no arguments and returns
 *           nothing.
 *           The record pointer at 0x8014638C is read once into a saved register
 *           before the byte copy, because the store through the work pointer may
 *           alias it; the shared battle record pointer at 0x801EB4E0 and the
 *           work pointer cell at 0x1F800044 are read in the same prologue, and
 *           record byte 0x08 is copied into work byte 0x08.
 *           The work motion triple at 0xC, 0x10 and 0x14 is then selected from
 *           the acting battler byte 0x80146374. A battler index below 3 takes
 *           word 0xC = 0x4000, word 0x10 = 0 and word 0x14 = 0x1000000. A
 *           battler index of 3 or more takes the kind byte of the 0x118-byte
 *           stride local panel record at 0x801EB6AC indexed by that index minus
 *           three: kind 0x9A takes 0x10000 and 0x2000000, kind 0xB5 takes
 *           0xFFFF2000 and 0x3800000, and every other kind takes 0x2000 and
 *           0x1C00000. Every arm clears word 0x10 and reads the pointer cell for
 *           its own work base, and the shared word 0x10 and word 0x14 stores
 *           cross-jump into one site fed by the arm register.
 *           The dispatch shape is evidence for a switch, not for a chain: the
 *           original tests kind 0x9A, then narrows with slti 0x9B into the
 *           default block, then tests 0xB5, and only a switch whose low case is
 *           also bound to the default block produces that range test. The
 *           compare of that low case is dropped because both of its edges reach
 *           the same block, so the bytes do not discriminate its value; 0x99 is
 *           the label this body uses, and the group it shares with default is
 *           the whole of its contribution.
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
 *           pair table at 0x801F0A38, clears work byte 0xB, sets work byte 0x9
 *           to 0x10 and work byte 0xA to 1, and advances work byte 0x2. The four
 *           table stores and the byte 0xB store share one work base while the
 *           three later byte stores each re-read the pointer cell, which is why
 *           only those three carry a fresh lui plus lw pair. The 0x18-byte frame
 *           keeps the return address at 0x14 and the saved record pointer at
 *           0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkMotionFromBattlerClassThenSetHeadingToward8014638CPublishHalfwordPairAndAdvanceWorkByte2BmagicMagic04403_801EEEFC(void) {
  u8 *work;
  u8 *headingWork;
  u8 *pairWork;
  u8 *partyWork;
  u8 *kind9aWork;
  u8 *kindB5Work;
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
    *(u32 *)(partyWork + 0x14) = 0x1000000;
  } else {
    switch (D_801EB6AC[D_80146374 - 3].kind) {
    case 0x99:
    default:
      defaultWork = g_battle_work;
      *(u32 *)(defaultWork + 0xC) = 0x2000;
      *(u32 *)(defaultWork + 0x10) = 0;
      *(u32 *)(defaultWork + 0x14) = 0x1C00000;
      break;
    case 0x9A:
      kind9aWork = g_battle_work;
      *(u32 *)(kind9aWork + 0xC) = 0x10000;
      *(u32 *)(kind9aWork + 0x10) = 0;
      *(u32 *)(kind9aWork + 0x14) = 0x2000000;
      break;
    case 0xB5:
      kindB5Work = g_battle_work;
      *(u32 *)(kindB5Work + 0xC) = 0xFFFF2000;
      *(u32 *)(kindB5Work + 0x10) = 0;
      *(u32 *)(kindB5Work + 0x14) = 0x3800000;
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
  D_801F0A38[pairWork[4]].unk_00 = *(u16 *)(pairWork + 0x2E);
  D_801F0A38[pairWork[4]].unk_02 = *(u16 *)(pairWork + 0x30);
  D_801F0A38[pairWork[4]].unk_04 = *(u16 *)(pairWork + 0x2E);
  D_801F0A38[pairWork[4]].unk_06 = *(u16 *)(pairWork + 0x30);
  pairWork[0xB] = 0;
  g_battle_work[9] = 0x10;
  g_battle_work[0xA] = 1;
  g_battle_work[2]++;
}
