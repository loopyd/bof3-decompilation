#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * with its own lui 0x1F80 plus lw 0x44 pair at each read, because a store
 * through the loaded pointer may alias the cell, so the cell is named rather
 * than written as a bare scratchpad constant. Bound at 0x1F800044 by this
 * target map. The exact same-target sibling
 * seedWorkMotionFromBattlerClassThenSetHeadingToward8014638CPublishHalfwordPair
 * AndAdvanceWorkByte2BmagicMagic17303_801EEEEC declares the same cell the same
 * way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Record pointer cell at 0x8014638C in main RAM, outside this overlay image, so
 * it lives in the main executable both this payload and the sibling payloads
 * run against and keeps the same name everywhere. This body reads words 0x34,
 * 0x38 and 0x3C of the record it points to; the role of the record is not
 * proven, so the name stays address-anchored, exactly as the exact
 * same-target sibling above declares it. Bound through this target generated
 * undefined_syms_auto.txt row. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Shared battle target byte at 0x80146384 in main RAM, forwarded with the flag
 * word 0x10 to the battler status helper when this step raises that flag. The
 * exact bmagic sibling
 * raiseBattleTargetFlag10WhenWorkByte9SetThenAdvanceWorkByte1BmagicMagic10003_
 * 801EED9C forwards the same cell to the same helper. Bound through this
 * target generated undefined_syms_auto.txt row. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle record pointer at 0x801EB4E0 in main RAM, outside this overlay
 * image. The pointer is published into the queued action slot at +0x74 and that
 * record byte 0x0B is the count this body increments; the cell is read twice,
 * once for the publish and once before that increment, so each read keeps its
 * own local. Bound through this target generated undefined_syms_auto.txt row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Action-slot pool at 0x801EC330, whose 0x78-byte stride these accesses scale by
 * with sll 4, subu and sll 3. The original reaches each field as
 * lui $at,%hi(ADDR) plus addu $at,$at,index*0x78 plus sb|sw %lo(ADDR)($at)
 * rather than through one shared record base, so every addressed field keeps
 * its own flat symbol: byte +0x01 and the words at +0x34, +0x38, +0x3C and
 * +0x74 of one entry. include/bof3/battle/battle03_internal.h declares the
 * reviewed aggregate view of the same pool as Battle03SlotStore, and the exact
 * bmagic sibling
 * queueSlotSetByte1PublishRecordPointerCopyWorkWordTripletEveryFourthTickWhile
 * ByteABelow16ThenAdvanceWorkByte2At16BmagicMagic04403_801EF320 declares these
 * five flat rows and stores into them in this order. Bound through this target
 * generated undefined_syms_auto.txt rows. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC364[]; /* @source 0x801EC364 @kind data */
extern u8 D_801EC368[]; /* @source 0x801EC368 @kind data */
extern u8 D_801EC36C[]; /* @source 0x801EC36C @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Point helper of the main executable: it takes the four-word point this body
 * projects from the 0x8014638C record, whose fourth word is never assigned, and
 * the size word. The exact bmagic siblings
 * drawBattlerRecordPointSnapPositionWhenSlotUnavailableAndOnRangeHitRaiseFlag
 * 2000ForwardSlotSetHeadingTowardBattleRecordAndAdvanceWorkByte2BmagicMagic01
 * 200_801EF188 and raiseWorkByte9ByFourWhenBelow10InvokePointHelperStoreHeading
 * Toward8014638CAndAdvanceWorkByte2WhenSlotByte2IsTwoBmagicMagic17203_801EF650
 * declare the same five-argument shape against the same projection and pass a
 * fourth element they never set. */
extern void func_800AF3A0(s32 x, s32 y, s32 z, s32 depth, s32 size); /* @source 0x800AF3A0 */

/* Two-argument angle helper of the main executable. The exact bmagic siblings
 * named above call it with the word 0x34 and 0x38 differences of the same two
 * records and store the answer into work word 0xC. */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */

/* Point-range helper of the main executable: it reports whether the work object
 * lies within the bound it is handed of the record, which this body passes as
 * the bound word 0x8000. The same-target siblings
 * drawBattlerRecordPointSnapPositionWhenSlotUnavailableAndOnRangeHitRaiseFlag
 * 2000ForwardSlotSetHeadingTowardBattleRecordAndAdvanceWorkByte2BmagicMagic01
 * 200_801EF188 and
 * forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagic
 * Magic17303_801EED90 test its answer the same way. */
extern u32 func_800AF66C(u8 *record, u32 size); /* @source 0x800AF66C */

/* Service helpers of the main executable called unconditionally by this step,
 * and the battler status helper that raises one flag of the shared target byte.
 * The no-argument service shape is the one the exact same-target sibling
 * dispatchStateByte2Table09a0BmagicMagic17303_801F0488 calls, and the status
 * helper keeps the two-argument shape every bmagic lift of this family uses. */
extern void func_800AF764(void); /* @source 0x800AF764 */
extern void func_800A9610(u8 arg0, s32 arg1); /* @source 0x800A9610 */

/* Action-slot allocator of the main executable: it returns the index of the
 * entry it claimed, zero-extended here, or 0xFF when the pool holds no free
 * entry. This body uses the returned index unconditionally, without the 0xFF
 * test the sibling lift named above performs. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Overlay step that follows this body in the payload and stays an asm boundary
 * of this target, so it keeps its raw name and needs no map row; this target
 * map binds the address through the boundary label. */
extern void func_801F02B0(void); /* @source 0x801F02B0 */

/* @source 0x801EF148
 * @behavior Entry 3 of the ten-entry code-pointer run based at 0x801F0988,
 *           indexed by the scratchpad work-object byte 0x1 by this overlay
 *           step dispatcher
 *           dispatchStateByte1Table0988BmagicMagic17303_801EEDF0, whose word 3
 *           holds 0x801EF148; no jal of this payload reaches this address, so
 *           the table row proves the boundary and the no-argument ABI. It takes
 *           no arguments, returns nothing, and the 0x30-byte frame keeps the
 *           return address at 0x2C and the saved record pointer at 0x28.
 *           The cell at 0x1F800044 is read once before the frame is created and
 *           the body raises work byte 0x0A by one while that byte is below 8,
 *           the bound the original compares with sltiu and the sum it stores
 *           with sb.
 *           It then loads the record pointer cell at 0x8014638C once into a
 *           saved register and builds a stack point triple from that record:
 *           word 0x34 and word 0x38 shifted right by nine and biased by minus
 *           0x4000, and word 0x3C plus 0x800000 shifted right by 17. The three
 *           words sit at 0x18, 0x1C and 0x20 of the frame and are passed to the
 *           point helper func_800AF3A0 together with the fourth frame word at
 *           0x24, which the body never writes, and the size literal 0xC0 stored
 *           at 0x10 as the helper fifth argument.
 *           After the call it re-reads the cell and copies work word 0xC into
 *           work word 0x10, the heading of the previous frame into the
 *           previous-heading slot, while the same read supplies work words 0x34
 *           and 0x38 as the subtrahends of the heading difference. The heading
 *           toward the 0x8014638C record comes from func_8017A138 of that
 *           record word 0x34 minus work word 0x34 and record word 0x38 minus
 *           work word 0x38, the second difference filling the call delay slot,
 *           and the answer replaces work word 0xC.
 *           The point-range helper func_800AF66C is then called with the same
 *           record and the bound word 0x8000. On a hit the body re-reads the
 *           cell and copies record words 0x34 and 0x38 into the same work words,
 *           then raises work byte 0x2; the byte store of that arm is what the
 *           original leaves in the jump delay slot. Off a hit it samples the
 *           heading difference instead: the low 12 bits of work word 0x10
 *           minus the low 12 bits of work word 0xC, stored back into work word
 *           0x10 and negated there when it is negative with nor plus one, and
 *           only while that truncated magnitude minus 0x601 is below 0x3FF in an
 *           unsigned compare does it copy the record words and raise work byte
 *           0x2 exactly as the hit arm does, through one further cell read that
 *           serves both the test and the stores.
 *           It then calls the shared service func_800AF764 and the overlay step
 *           func_801F02B0 unconditionally, re-reads the cell and, while work
 *           byte 0x2 is not 1 and work byte 0x4 is zero, forwards the shared
 *           battle target byte 0x80146384 with the flag word 0x10 to the battler
 *           status helper func_800A9610 so that this step raises that flag.
 *           Last, while work byte 0x2 is exactly 2, it claims an action slot
 *           through func_801E590C(1, 0x18), scales the returned index by the
 *           0x78-byte slot stride and fills five fields of the entry: byte
 *           +0x01 becomes 1, the pointer word at +0x74 receives the shared battle
 *           record pointer read from 0x801EB4E0, the work words 0x34, 0x38 and
 *           0x3C are copied into the entry words at the same offsets through the
 *           same cell read, and the record pointer is re-read so that its byte
 *           0x0B count is raised by one. The allocator answer is used without a
 *           0xFF test, so a full pool would index one entry past the pool.
 *
 *           The prefix is one body with a near-identical sibling at 0x801EF138
 *           in the magic044 #3 payload, whose 0x1E4-byte span holds this body up
 *           to the flag raise and then stops at its epilogue: a word comparison
 *           of the two spans differs in exactly three words, the sltiu bound
 *           0x10 against this span 8, and the absolute words of the j to the
 *           join and of the jal to the in-payload step at 0x801F02A0, each
 *           shifted by the 0x10 that separates the two spans, so relative
 *           branches, every literal, the 0xC0 and 0x8000 bounds and the
 *           0x601/0x3FF window are equal and that sibling is shape evidence for
 *           the prefix rather than a clone. Its slot-publish block is absent and
 *           this span draws that block, its store order and its operand forms
 *           from the exact bmagic sibling
 *           queueSlotSetByte1PublishRecordPointerCopyWorkWordTripletEveryFourth
 *           TickWhileByteABelow16ThenAdvanceWorkByte2At16BmagicMagic04403_801EF3
 *           20, whose gate and allocator test differ from this span own bytes. A
 *           byte search of the 396 payloads under out/binaries finds no second
 *           copy of this 0x290-byte span, so the clone lever was declined. The
 *           roles of work bytes 0x2, 0x4 and 0x0A, of work words 0xC, 0x10, 0x34,
 *           0x38 and 0x3C, of the 0x8014638C record field set and of the queued
 *           slot fields remain unestablished, so they stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawRecordPointSetHeadingToward8014638CAdvanceWorkByte2OnPointRangeHitRaiseFlag10ThenQueueActionSlotBmagicMagic17303_801EF148(void) {
  u8 *work;
  u8 *hitWork;
  u8 *record;
  u8 *slotRecord;
  u8 *slotWork;
  u8 *recordCount;
  s32 point[4];
  s32 heading;
  s32 diff;
  u32 slot;
  u32 offset;

  if (g_battle_work[0xA] < 8) {
    g_battle_work[0xA] += 1;
  }
  record = D_8014638C;
  point[0] = (*(s32 *)(record + 0x34) >> 9) - 0x4000;
  point[1] = (*(s32 *)(record + 0x38) >> 9) - 0x4000;
  point[2] = (*(s32 *)(record + 0x3C) + 0x800000) >> 17;
  func_800AF3A0(point[0], point[1], point[2], point[3], 0xC0);
  *(s32 *)(g_battle_work + 0x10) = *(s32 *)(g_battle_work + 0xC);
  heading = func_8017A138(*(s32 *)(record + 0x34) - *(s32 *)(g_battle_work + 0x34),
                          *(s32 *)(record + 0x38) - *(s32 *)(g_battle_work + 0x38));
  *(s32 *)(g_battle_work + 0xC) = heading;
  if (func_800AF66C(record, 0x8000)) {
    hitWork = g_battle_work;
    *(s32 *)(hitWork + 0x34) = *(s32 *)(record + 0x34);
    *(s32 *)(hitWork + 0x38) = *(s32 *)(record + 0x38);
    hitWork[2] += 1;
  } else {
    diff = (*(s32 *)(g_battle_work + 0x10) & 0xFFF) - (*(s32 *)(g_battle_work + 0xC) & 0xFFF);
    *(s32 *)(g_battle_work + 0x10) = diff;
    if (diff < 0) {
      *(s32 *)(g_battle_work + 0x10) = ~diff + 1;
    }
    work = g_battle_work;
    if (*(u32 *)(work + 0x10) - 0x601u < 0x3FFu) {
      *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34);
      *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38);
      work[2] += 1;
    }
  }
  func_800AF764();
  func_801F02B0();
  work = g_battle_work;
  if (work[2] != 1 && work[4] == 0) {
    func_800A9610(D_80146384, 0x10);
  }
  if (g_battle_work[2] == 2) {
    slot = (u8)func_801E590C(1u, 0x18u);
    offset = slot * 0x78;
    slotRecord = D_801EB4E0;
    D_801EC331[offset] = 1;
    slotWork = g_battle_work;
    *(u32 *)&D_801EC3A4[offset] = (u32)slotRecord;
    *(u32 *)&D_801EC364[offset] = *(u32 *)(slotWork + 0x34);
    *(u32 *)&D_801EC368[offset] = *(u32 *)(slotWork + 0x38);
    recordCount = D_801EB4E0;
    *(u32 *)&D_801EC36C[offset] = *(u32 *)(slotWork + 0x3C);
    recordCount[0xB] += 1;
  }
}
