#include "bof3/bof3.h"

extern u8 D_80145FD0[]; /* @source 0x80145FD0 @kind unknown */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind unknown */
extern u8 D_80144F5B; /* @source 0x80144F5B @kind unknown */
extern s16 D_80145FFE; /* @source 0x80145FFE @kind unknown */
extern s16 D_80146000; /* @source 0x80146000 @kind unknown */
extern u8 D_801F3514[]; /* @source 0x801F3514 @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F3118
 * @behavior Publishes the second shared work base D_80145FD0 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 1 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = the per-party byte row at 0x801F3514
 *           indexed by the second shared party byte D_80144F5B and halfwords
 *           +0x2E/+0x30 = D_80145FFE/D_80146000; unless the call reports 0xFF
 *           the returned record index is stored in work byte +0x0B. Takes no
 *           arguments and returns nothing. No jal reaches this address: its
 *           only in-image reference is the code-pointer word at 0x801F52C4, the
 *           sixth entry of the handler run 0x801F52B0 that lists this overlay's
 *           entry points in address order inside this payload's T_801F5204
 *           textbin blob, so the state dispatcher reaches it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this 0x80-byte body is instruction
 * identical to the exact sibling
 * publishSecondWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld02Area09413_801F2F50
 * at 0x801F2F50 of this same payload except for two shipped words (payload cmp
 * of the two spans: the record-kind immediate at +0x2C, 1 here against 3 there,
 * and the mode-row lbu displacement low half at +0x3C, 0x3514 here against
 * 0x3504 there), so statement order, operand forms, declaration style and the
 * reload of the scratchpad cell after the call are taken from that sibling and
 * only this function's own kind and mode-row literals are written here. The
 * published work base D_80145FD0, the party-id byte D_80144F5B it indexes with
 * and the two forwarded +0x2E/+0x30 halfwords are the same here, which is what
 * places D_80145FD0 second of the three work bases D_80145E90/D_80145FD0/
 * D_80146110 and D_80144F5B second of the party-id bytes at D_80144F5A. The
 * next range 0x801F3198 carries this body's own kind and mode row against the
 * third work base, differing in four words of shipped literals, so the family
 * encodes its table base and record kind in its instruction words and no member
 * is byte-identical to another; a scan of the 0x80-byte span against every one
 * of the 398 extracted payloads under out/binaries found no match at all, so
 * the byte-identical clone lever does not apply. This body's mode row starts
 * 0x10 bytes into this payload's 0x801F3504 span, which the generated name
 * spells as a function although the original only ever loads from it as a
 * table, and it binds as a u8 array at its own word through the generated
 * symbol row for 0x801F3514. The scratchpad cell is re-read after the call, so
 * the index lands in the record the callee saw rather than in the u8* copy
 * published before it.
 */
void publishSecondWorkBaseAndSeedFrontEndRecordKind1FromModeTableWorld02Area09413_801F3118(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80145FD0;
  result = func_8015CB18(1, 0, D_801F3514[D_80144F5B], D_80145FFE, D_80146000);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
