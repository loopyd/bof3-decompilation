#include "bof3/bof3.h"

extern u8 D_80145FD0[]; /* @source 0x80145FD0 @kind unknown */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80144F5B; /* @source 0x80144F5B @kind unknown */
extern s16 D_80145FFE; /* @source 0x80145FFE @kind unknown */
extern s16 D_80146000; /* @source 0x80146000 @kind unknown */
extern u8 D_801F398C[]; /* @source 0x801F398C @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F34D4
 * @behavior Publishes the second shared work base D_80145FD0 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 4 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = the mode-indexed byte of the table at
 *           0x801F398C (indexed by the shared party-id byte D_80144F5B) and
 *           halfwords +0x2E/+0x30 = D_80145FFE/D_80146000; unless the call
 *           reports 0xFF the returned record index is stored in work byte
 *           +0x0B. Takes no arguments and returns nothing. No jal reaches this
 *           address: its only in-image reference is the code-pointer word at
 *           0x801F5C60, index 11 of the 32-entry state-pointer run at
 *           0x801F5C34 in this payload's D_801F398C textbin blob, which lists
 *           this overlay's handlers in address order, so the state dispatcher
 *           reaches it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: a payload cmp of this 0x80-byte span at
 * payload offset 0x8D4 against the exact sibling
 * publishSecondWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld02Area09413_801F32B0
 * at 0x801F32B0 of emi/world02/area094/13 leaves 126 of 128 bytes equal. The
 * two differing bytes are the low half of the mode-row `lbu` displacement word
 * at +0x3C, 0x398C here against 0x350C there, which is this payload's own mode
 * row. The same cmp against the kind 4 second-base members of
 * emi/world02/area077/14 (0x801F5E04) and emi/world01/area068/13 (0x801F3A18)
 * leaves the same 126 of 128 bytes equal with 0x62EC and 0x4060 in that one
 * word, so all four spans are one body and only the mode-row base is target
 * specific. Statement order, operand forms, declaration style and the re-read
 * of the scratchpad cell after the call therefore come from that first sibling,
 * and only the mode-row binding is written from these bytes. The published work
 * base D_80145FD0, the party-id byte D_80144F5B and the two forwarded halfwords
 * D_80145FFE/D_80146000 are already identical to those siblings: D_80145FD0 is
 * the second of the three 0x140-byte-stride work bases
 * D_80145E90/D_80145FD0/D_80146110 and D_80144F5B the second of the three
 * shared party-id bytes at 0x80144F5A through 0x80144F5C, so base, selected
 * table byte and the +0x2E/+0x30 halfwords all belong to one party member. The
 * table is this payload's byte-entry mode table at the head of the D_801F398C
 * textbin region, which the neighbouring kind 4 seeds
 * 0x801F3454 (first base) and 0x801F3554 (third base) index the same way, so it
 * binds at the region start. The scratchpad cell is re-read after the call, so
 * the index lands in the record the callee saw rather than in the u8* copy
 * published before it.
 */
void publishSecondWorkKind4(
    void) {
  u8 result;

  D_1F800044 = D_80145FD0;
  result = func_8015CB18(4, 0, D_801F398C[D_80144F5B], D_80145FFE, D_80146000);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
