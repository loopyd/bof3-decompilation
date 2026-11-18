#include "bof3/bof3.h"

extern u8 D_80145E90[]; /* @source 0x80145E90 @kind unknown */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind unknown */
extern u8 D_80144F5A; /* @source 0x80144F5A @kind unknown */
extern s16 D_80145EBE; /* @source 0x80145EBE @kind unknown */
extern s16 D_80145EC0; /* @source 0x80145EC0 @kind unknown */
extern u8 D_801F350C[]; /* @source 0x801F350C @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F3050
 * @behavior Publishes the shared work base D_80145E90 as the scratchpad work
 *           cursor at 0x1F800044 and then seeds one front-end record of kind 4
 *           through the shared allocator/seeder func_8015CB18 with word +0x0C =
 *           0, word +0x10 = the mode-indexed byte of the table at 0x801F350C
 *           (indexed by the leading shared party-id byte D_80144F5A) and
 *           halfwords +0x2E/+0x30 = D_80145EBE/D_80145EC0; unless the call
 *           reports 0xFF the returned record index is stored in work byte
 *           +0x0B. Takes no arguments and returns nothing. No jal reaches this
 *           address: its only in-image reference is the code-pointer word at
 *           0x801F52BC, the fourth entry of the handler run 0x801F52B0 that
 *           lists 0x801F2ED0/0x801F2F50/0x801F2FD0/0x801F3050 in address order
 *           inside this payload's T_801F5204 textbin blob, so the state
 *           dispatcher reaches it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the 0x80-byte range is instruction
 * identical to the exact sibling
 * publishWorkBaseAndSeedFrontEndRecordKind2FromModeTableWorld02Area09413_801F2ED0
 * at 0x801F2ED0 of this same payload except for two words of shipped literals
 * (payload cmp of the two spans: the record-kind immediate at +0x2C, 4 here
 * against 2 there, and the mode-row lbu displacement at +0x3C, 0x350C here
 * against 0x3504 there), so statement order, operand forms and declaration
 * style are taken from that sibling and only this function's own kind and
 * mode-row binding are written from its bytes. The published work base, the
 * shared party-id byte D_80144F5A it indexes the row with, the two forwarded
 * +0x2E/+0x30 halfwords and the scratchpad publish/re-read spelling are already
 * identical to that sibling, so they are unchanged here. The mode row
 * D_801F350C is the second of the two adjacent byte rows this payload's family
 * reads - 0x801F3504 for the kind 2 and kind 3 seeds, 0x801F350C for the kind 4
 * seeds 0x801F3050/0x801F32B0/0x801F3330 - so it sits inside this payload's
 * own 0x801F3504 span, which the generated name spells as a function although
 * the original only ever loads from it as a table, and binds as a u8 array at
 * its own word. The scratchpad cell is re-read after the call, so the index
 * lands in the record the callee saw rather than in the u8* copy published
 * before it.
 */
void publishWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld02Area09413_801F3050(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80145E90;
  result = func_8015CB18(4, 0, D_801F350C[D_80144F5A], D_80145EBE, D_80145EC0);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
