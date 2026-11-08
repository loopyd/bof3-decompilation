#include "bof3/bof3.h"

/* First shared work base this overlay publishes as the scratchpad cursor.
 * @source 0x80145E90 @kind unknown
 */
extern u8 D_80145E90[];
/* Scratchpad work-pointer cell: the original re-materialises the cell for the
 * publish and again after the call, so the cell keeps its volatile form.
 * @source 0x1F800044 @kind data
 */
extern u8* volatile D_1F800044;
/* Shared party selector byte the seeders of this overlay read as their
 * mode-table index (0xF5A/0xF5B/0xF5C).
 * @source 0x80144F5A @kind unknown
 */
extern u8 D_80144F5A;
/* The published base's own +0x2E halfword. @source 0x80145EBE @kind unknown */
extern s16 D_80145EBE;
/* The published base's own +0x30 halfword. @source 0x80145EC0 @kind unknown */
extern s16 D_80145EC0;
/* Mode table this payload's kind-1/2/3 seeders read: byte 0x144 inside the
 * 0x801F5724 tail blob, the row four of the six seeders of this overlay share.
 * @source 0x801F5868 @kind table
 */
extern u8 D_801F5868[];

/* Shared front-end record allocator and seeder at 0x8015CB18: takes the next
 * free 0x74-byte record index from func_8019601C and fills record byte +0 = 1,
 * byte +5 = 6, byte +6 = arg0, word +0x0C = (s8)arg1, word +0x10 = (s8)arg2,
 * halfword +0x2E = arg3 and halfword +0x30 = arg4 before returning that index,
 * or returns 0xFF without touching the table when it is full.
 */
u8 func_8015CB18(s32 arg0, s32 arg1, u8 arg2, s16 arg3, s16 arg4);

/* @source 0x801F5200
 * @behavior Publishes the first shared work base D_80145E90 as the scratchpad
 *           work cursor at 0x1F800044 and then seeds one front-end record of
 *           kind 1 through the shared allocator/seeder func_8015CB18 with word
 *           +0x0C = 0, word +0x10 = the selector-indexed byte
 *           D_801F5868[D_80144F5A] and halfwords +0x2E/+0x30 =
 *           D_80145EBE/D_80145EC0, which are the published base's own
 *           +0x2E/+0x30; unless the call reports 0xFF the returned record index
 *           is stored in work byte +0x0B, the byte the byte-2 handlers of this
 *           overlay read. Takes no arguments and returns nothing. No jal
 *           reaches this address: its only in-image reference is the sixth
 *           code-pointer word of the run at 0x801F6330 in this payload's tail
 *           data blob, so the overlay reaches it indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this 0x80-byte body is
 * instruction-identical to the exact sibling of this target at 0x801F5114
 * except for exactly two words, the record-kind literal (1 here, 4 there) and
 * the mode-table displacement (0x5868 here, 0x5874 there), so statement order,
 * operand forms, declaration style and the re-read of the scratchpad cell
 * after the call are taken from that sibling, and only this function's own
 * literals and symbol bindings are written here. The published work base, the
 * selector byte, the two forwarded +0x2E/+0x30 halfwords and the store of the
 * returned index are identical to that sibling. The sibling's own notes record
 * this split: 0x801F5114 alone reads the D_801F5874 row while the other five
 * seeders of this overlay read D_801F5868, and the kind-1 members are this
 * address, 0x801F53EC and 0x801F546C. All five siblings are still asm rows and
 * each carries a different literal pair, so no byte-identical twin exists in
 * any target and the clone lever does not apply. The table at 0x801F5868 sits
 * 0x144 bytes inside the D_801F5724 textbin run and keeps the generated
 * undefined_syms_auto row because four asm siblings still read it.
 */
void publishWorkBaseAndSeedFrontEndRecordKind1FromModeTableWorld03Area13413_801F5200(
    void) {
  u8 result;

  D_1F800044 = D_80145E90;
  result = func_8015CB18(1, 0, D_801F5868[D_80144F5A], D_80145EBE, D_80145EC0);
  if (result != 0xFF) {
    D_1F800044[0xB] = result;
  }
}
