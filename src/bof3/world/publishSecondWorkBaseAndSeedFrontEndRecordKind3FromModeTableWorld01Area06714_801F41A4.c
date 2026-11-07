#include "bof3/bof3.h"

extern u8 D_80145FD0[]; /* @source 0x80145FD0 @kind unknown */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80144F5B; /* @source 0x80144F5B @kind unknown */
extern s16 D_80145FFE; /* @source 0x80145FFE @kind unknown */
extern s16 D_80146000; /* @source 0x80146000 @kind unknown */
extern u8 D_801F4644[]; /* @source 0x801F4644 @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F41A4
 * @behavior Publishes the second shared work base D_80145FD0 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 3 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = D_801F4644[D_80144F5B] and halfwords
 *           +0x2E/+0x30 = D_80145FFE/D_80146000; unless the call reports 0xFF
 *           the returned record index is stored in work byte +0x0B. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this 0x80-byte body is instruction
 * identical to the exact sibling
 * publishWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld01Area06714_801F4124
 * at 0x801F4124 in this same payload except for the four shipped symbol
 * immediates - the published work base, the shared party-id byte used to index
 * the mode table, and the two halfwords forwarded to the seeder - so its
 * statement order, operand forms, offset spelling and declaration style are
 * taken from that sibling and only this function's own bindings are written
 * here. Both halves of the published pair belong to the same party member:
 * D_80145FD0 is the second of the three 0x140-byte-stride work bases
 * D_80145E90/D_80145FD0/D_80146110 that the boss overlay installer 0x800C1B0C
 * walks as its three channel groups, D_80144F5B is the second of the three
 * shared party-id bytes at D_80144F5A, and D_80145FFE/D_80146000 are that
 * base's own +0x2E/+0x30. The two bases share one mode table in this payload,
 * so the party-id byte, not the table base, selects the record kind byte at
 * 0x801F4644. Nothing was cloned: the ranges differ in their literals, so the
 * sibling is shape evidence only.
 */
void publishSecondWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld01Area06714_801F41A4(
    void) {
  u8 result;

  D_1F800044 = D_80145FD0;
  result = func_8015CB18(3, 0, D_801F4644[D_80144F5B], D_80145FFE, D_80146000);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
