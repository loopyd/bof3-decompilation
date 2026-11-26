#include "bof3/bof3.h"

extern u8 D_80145E90[]; /* @source 0x80145E90 @kind unknown */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80144F5A; /* @source 0x80144F5A @kind unknown */
extern s16 D_80145EBE; /* @source 0x80145EBE @kind unknown */
extern s16 D_80145EC0; /* @source 0x80145EC0 @kind unknown */
extern u8 tablePartyModeBytes[]; /* @source 0x801F398C @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F3454
 * @behavior Publishes the shared work base D_80145E90 as the scratchpad work
 *           cursor at 0x1F800044 and then seeds one front-end record of kind 4
 *           through the shared allocator/seeder func_8015CB18 with word +0x0C =
 *           0, word +0x10 = the mode-indexed byte of the table at 0x801F398C
 *           (indexed by the leading shared party-id byte D_80144F5A) and
 *           halfwords +0x2E/+0x30 = D_80145EBE/D_80145EC0; unless the call
 *           reports 0xFF the returned record index is stored in work byte
 *           +0x0B. Takes no arguments and returns nothing. No jal reaches this
 *           address: its only in-image reference is the code-pointer word at
 *           0x801F5C5C, index 10 of the 32-entry state-pointer run at 0x801F5C34
 *           that lists this overlay's handlers in address order inside the
 *           payload's T_801F398C textbin blob, so the state dispatcher reaches
 *           it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: a payload cmp of the 0x80-byte span
 * against the exact sibling
 * publishWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld02Area09413_801F3050
 * at 0x801F3050 in emi/world02/area094/13 leaves 126 of 128 bytes equal. The
 * single differing word is offset +0x3C, the low half of the mode-row lbu
 * displacement: 0x398C here against 0x350C there, which is this payload's own
 * mode row. Statement order, operand forms, declaration style and the re-read
 * of the scratchpad cell after the call therefore come from that sibling, and
 * only the mode-row binding is written from these bytes. The published work
 * base D_80145E90, the party-id byte D_80144F5A and the two forwarded
 * halfwords D_80145EBE/D_80145EC0 are already identical to that sibling, which
 * is the first-base member of the same kind-4 group: D_80145E90 is the first of
 * the three 0x140-byte-stride work bases D_80145E90/D_80145FD0/D_80146110 and
 * D_80144F5A the first of the three shared party-id bytes at 0x80144F5A
 * through 0x80144F5C, so base, selected table byte and the +0x2E/+0x30
 * halfwords all belong to one party member. The table is this payload's
 * byte-entry mode table at the head of the tablePartyModeBytes textbin region, which the
 * neighbouring handlers 0x801F34D4 and 0x801F3554 index the same way, so it
 * binds at the region start. The scratchpad cell is re-read after the call, so
 * the index lands in the record the callee saw rather than in the u8* copy
 * published before it.
 */
void publishBaseSeedRecord(
    void) {
  u8 result;

  D_1F800044 = D_80145E90;
  result = func_8015CB18(4, 0, tablePartyModeBytes[D_80144F5A], D_80145EBE, D_80145EC0);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
