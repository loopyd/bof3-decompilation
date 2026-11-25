#include "bof3/bof3.h"

extern u8 D_80146110[]; /* @source 0x80146110 @kind unknown */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind unknown */
extern u8 D_80144F5C; /* @source 0x80144F5C @kind unknown */
extern s16 D_8014613E; /* @source 0x8014613E @kind unknown */
extern s16 D_80146140; /* @source 0x80146140 @kind unknown */
extern u8 D_801F3514[]; /* @source 0x801F3514 @kind table */

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F3198
 * @behavior Publishes the third shared work base D_80146110 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 1 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = the per-party byte row at 0x801F3514
 *           indexed by the third shared party byte D_80144F5C and halfwords
 *           +0x2E/+0x30 = D_8014613E/D_80146140; unless the call reports 0xFF
 *           the returned record index is stored in work byte +0x0B. Takes no
 *           arguments and returns nothing. No jal reaches this address: its
 *           only in-image reference is the code-pointer word at 0x801F52C8, the
 *           seventh entry of the handler run 0x801F52B0 that lists this
 *           overlay's entry points in address order inside this payload's
 *           T_801F5204 textbin blob, so the state dispatcher reaches it
 *           indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this 0x80-byte body is instruction
 * identical to the exact sibling
 * stateSeedMode3C
 * at 0x801F2FD0 of this same payload except for two shipped bytes (payload cmp
 * of the two spans: the record-kind immediate at +0x2C, 1 here against 3 there,
 * and the low half of the mode-row lbu displacement at +0x3C, 0x3514 here
 * against 0x3504 there), so statement order, operand forms, declaration style
 * and the reload of the scratchpad cell after the call are taken from that
 * sibling and only this function's own kind and mode-row literals are written
 * here. The published work base D_80146110, the party-id byte D_80144F5C it
 * indexes with and the two forwarded +0x2E/+0x30 halfwords D_8014613E/D_80146140
 * are the same here, which places this member third of the three work bases
 * D_80145E90/D_80145FD0/D_80146110 and D_80144F5C third of the party-id bytes at
 * D_80144F5A; the same body over the second base at 0x801F3118 carries this
 * record kind and mode row against D_80145FD0/D_80144F5B, so the family encodes
 * its table base and record kind in its instruction words and no member is
 * byte-identical to another. A scan of the 0x80-byte span against every one of
 * the 398 extracted payloads under out/binaries found this span only at its own
 * offset, so the byte-identical clone lever did not apply. This body's mode row
 * starts 0x10 bytes into this payload's 0x801F3504 span, which the generated
 * name spells as a function although the original only ever loads from it as a
 * table, and it binds as a u8 array at its own word through the generated
 * symbol row for 0x801F3514. The scratchpad cell is re-read after the call, so
 * the index lands in the record the callee saw rather than in the u8* copy
 * published before it.
 */
void stateSeedParty1C(
    void) {
  u8 result;

  workRecordCursor1F800044 = D_80146110;
  result = func_8015CB18(1, 0, D_801F3514[D_80144F5C], D_8014613E, D_80146140);
  if (result != 0xFF) {
    workRecordCursor1F800044[0xB] = result;
  }
}
