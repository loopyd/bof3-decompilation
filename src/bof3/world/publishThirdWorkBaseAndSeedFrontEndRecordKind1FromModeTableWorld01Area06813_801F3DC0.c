#include "bof3/bof3.h"

extern u8 D_80146110[]; /* @source 0x80146110 @kind unknown */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80144F5C; /* @source 0x80144F5C @kind unknown */
extern s16 D_8014613E; /* @source 0x8014613E @kind unknown */
extern s16 D_80146140; /* @source 0x80146140 @kind unknown */
extern u8 D_801F406C[]; /* @source 0x801F406C @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F3DC0
 * @behavior Publishes the third shared work base D_80146110 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 1 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = the party-indexed byte of this payload's
 *           byte table at 0x801F406C (indexed by the third shared party-id byte
 *           D_80144F5C) and halfwords +0x2E/+0x30 =
 *           D_8014613E/D_80146140; unless the call reports 0xFF the returned
 *           record index is stored in work byte +0x0B. Takes no arguments and
 *           returns nothing. No jal reaches this address: its only in-image
 *           reference is the sixth code-pointer word, at 0x801F4D54, of the
 *           nine-word handler run at 0x801F4D40 in this payload's D_801F3FE4
 *           textbin blob, so the state dispatcher that owns that run enters it
 *           indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this 0x80-byte body differs from the
 * exact sibling publishThirdWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld01Area06813_801F3A98
 * at 0x801F3A98 in this same payload, and from
 * publishSecondWorkBaseAndSeedFrontEndRecordKind1FromModeTableWorld01Area06813_801F3D40
 * at the preceding run entry 0x801F3D40, in exactly the two shipped bytes that
 * carry those functions' own literals: here the record kind is 1 rather than 4
 * and the `lbu` row base is 0x801F406C rather than 0x801F4060, while the
 * published work base, the party-id byte it indexes the row with, the two
 * forwarded +0x2E/+0x30 halfwords and the scratchpad publish/re-read spelling
 * are identical to the 0x801F3A98 sibling one kind earlier. Statement order,
 * operand forms and declaration style therefore follow that sibling, and only
 * this function's kind literal and table binding are written from its bytes.
 * The reload of the scratchpad cell after the call matters: the returned index
 * lands in the record the callee saw rather than in the u8* copy published
 * before it. The byte table at 0x801F406C is the kind-1 row of the same
 * mode-table block, 0xC bytes past the kind-4 row at 0x801F4060, and binds at
 * its own address the same way that sibling binds its row.
 */
void publishThirdWorkBaseAndSeedFrontEndRecordKind1FromModeTableWorld01Area06813_801F3DC0(
    void) {
  u8 result;

  D_1F800044 = D_80146110;
  result = func_8015CB18(1, 0, D_801F406C[D_80144F5C], D_8014613E, D_80146140);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
