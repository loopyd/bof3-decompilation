#include "bof3/bof3.h"

/* Published scratch work object of the shared main executable: the original
 * materialises this cell with a lui plus a 0x44 offset load and keeps the
 * pointer in one register for the three word reads below. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x80117864
 * @behavior Spawn step of this overlay's 0x800E4800 work table: it takes the
 * free record its caller has just obtained from
 * findFreeWorkRecordAt800E4800World01Area04913_801178C4 (the 16 records of
 * 0x28-byte stride whose lead byte is the claim flag) and seeds it from the
 * scratch work object published at the scratchpad pointer cell 0x1F800044. The
 * record's claim byte at offset 0 takes 1, its byte at 1 takes 0 - the
 * per-frame handler index this overlay's pass func_80117410 reads against the
 * handler table at 0x80118194 - its byte at 2 takes 0x20, its byte at 3 takes
 * 0xC0, its halfword at 0x24 takes 0x80, its byte at 0x26 takes 2, its word at
 * 0x1C is cleared, and the position triple at 0x04/0x08/0x0C takes the
 * published object's words 0x34/0x38/0x3C, the coordinate triple the sibling
 * drawWorkCursorQuadWorld03Area14514_801F3938 projects and
 * claimFreeWorkRecordSeedPositionAndFadeWorld04Area18813_801F3DE0 also copies
 * into a record. Both callers, func_801176B8 and func_80117768, publish the
 * record in D_801181DC and then overwrite the word at 0x1C with their own
 * value, so that clear is only the record's default. The renderer func_801170D8
 * tests the byte at 0x26 bitwise and substitutes the record's byte at 3 as one
 * of the three color components it passes to func_8014E5A0, so the 2 and the
 * 0xC0 written here select that channel. The roles of the bytes at 2 and the
 * halfword at 0x24 are not localised by this payload, so they stay described by
 * offset. Takes the record pointer in $a0, makes no call and returns nothing;
 * its 0x60 bytes are a leaf with no frame, and the original schedules the word
 * store at 0x0C into the return delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimWorkRecordSeedWorkPositionWorld01Area04913_80117864(u8* record) {
  u8* work;

  work = D_1F800044;
  *(u32*)(record + 0x04) = *(u32*)(work + 0x34);
  *(u32*)(record + 0x08) = *(u32*)(work + 0x38);
  *(u32*)(record + 0x0C) = *(u32*)(work + 0x3C);
  record[0] = 1;
  *(u16*)(record + 0x24) = 0x80;
  record[3] = 0xC0;
  record[0x26] = 2;
  *(u32*)(record + 0x1C) = 0;
  record[1] = 0;
  record[2] = 0x20;
}
