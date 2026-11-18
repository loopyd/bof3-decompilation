#include "bof3/bof3.h"

extern u8 D_80145FD0[]; /* @source 0x80145FD0 @kind unknown */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind unknown */
extern u8 D_80144F5B; /* @source 0x80144F5B @kind unknown */
extern s16 D_80145FFE; /* @source 0x80145FFE @kind unknown */
extern s16 D_80146000; /* @source 0x80146000 @kind unknown */
extern u8 D_801F350C[]; /* @source 0x801F350C @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F32B0
 * @behavior Publishes the second shared work base D_80145FD0 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 4 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = the mode-indexed byte of the table at
 *           0x801F350C (indexed by the shared party-id byte D_80144F5B) and
 *           halfwords +0x2E/+0x30 = D_80145FFE/D_80146000; unless the call
 *           reports 0xFF the returned record index is stored in work byte
 *           +0x0B. Takes no arguments and returns nothing. No jal reaches this
 *           address: its only in-image reference is the code-pointer word at
 *           0x801F52D4, the tenth entry of the handler run 0x801F52B0 inside
 *           this payload's T_801F5204 textbin blob, which lists this overlay's
 *           handlers in address order, so the state dispatcher reaches it
 *           indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the 0x80-byte range is instruction
 * identical to the exact sibling
 * publishSecondWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld02Area09413_801F2F50
 * at 0x801F2F50 of this same payload except for two words of shipped literals
 * (payload cmp of the two spans at their own payload offsets: the record-kind
 * immediate at +0x2C, 4 here against 3 there, and the mode-row lbu displacement
 * at +0x3C, 0x350C here against 0x3504 there), so statement order, operand
 * forms and declaration style are taken from that sibling and only this
 * function's own kind and mode-row binding are written from its bytes. The
 * published work base, the party-id byte D_80144F5B it indexes the row with,
 * the two forwarded +0x2E/+0x30 halfwords and the scratchpad publish/re-read
 * spelling are already identical to that sibling. The name comes from the
 * cross-target sibling
 * publishSecondWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld02Area07714_801F5E04,
 * which carries the same second work base D_80145FD0, the same second party-id
 * byte D_80144F5B, the same kind 4 and the same two forwarded halfwords, and
 * differs only in its own payload's mode-table base. The mode row D_801F350C is
 * the table the kind 4 seeds of this payload read (0x801F3504 serves the kind 2
 * and kind 3 seeds), so it binds as a u8 array at its own word inside the
 * 0x801F3504 span that the generated name spells as a function although the
 * original only ever loads from it as a table. The scratchpad cell is re-read
 * after the call, so the index lands in the record the callee saw rather than
 * in the u8* copy published before it.
 */
void publishSecondWorkBaseAndSeedFrontEndRecordKind4FromModeTableWorld02Area09413_801F32B0(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80145FD0;
  result = func_8015CB18(4, 0, D_801F350C[D_80144F5B], D_80145FFE, D_80146000);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
