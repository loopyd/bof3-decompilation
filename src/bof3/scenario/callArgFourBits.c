#include "bof3/bof3.h"

extern u8 D_80181B11;
extern u8 D_80181B15;
extern u8 D_80181B16;
extern u8 D_80181B12;
extern u8 D_8014496F[];

void func_801C1ABC(s32 arg0);

/* @source 0x801FDBF4
 * @behavior Word 4, the last word, of the five-slot code-pointer run at
 * 0x801FDFA8, which holds 0x801FDAFC, 0x801FDB34, 0x801FDB80, 0x801FDBCC and
 * this address at 0x801FDFB8. No in-image jal or j reaches the address, so the
 * handler runs only if something outside the payload calls that slot. In source
 * order it calls the shared main-image helper at 0x801C1ABC with the constant
 * 2, then clears bit 0 of the field-record flag byte at D_8014496F for four
 * records, taking the index in this order from the unsigned bytes at
 * 0x80181B11, 0x80181B15, 0x80181B16 and 0x80181B12. Each clear loads its
 * index byte, forms index times 0xA4 and rewrites that record's +7 byte as
 * itself with the low bit cleared; the two accesses of one statement
 * materialise the 0x8014 page in $at on their own. The 0xA4-byte stride is the
 * shared field-record table this tree anchors at 0x80144968, where 0x8014496F
 * is a record's +7 flag byte, and the four index bytes are elements 1, 5, 6 and
 * 2 of the shared main-RAM index array anchored at 0x80181B10 that the etc and
 * shop lifts already bind; their wider roles are not established by this body,
 * so the name keeps the field and the operation rather than a guessed one. It
 * takes no arguments, returns nothing, and its 0x18-byte frame only keeps $ra
 * across the one call, whose argument constant is materialised in the delay
 * slot. The flag-clear block is the shape of the exact sibling
 * clearTwoIndexedRecordFlagBitsThenInvokeHelperTwiceThenHelperScenarioScena0200_801FE1C0
 * (0x801FE1C0 of emi/scenario/scena02/00, which measured exact when this body
 * was seeded with two clears before its helper calls), so the call order, the
 * four index bytes and the constants here come from this span's own bytes. No
 * byte-identical copy of these 0x120 bytes exists in the 397 other payloads
 * under out/binaries.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callArgFourBits(
    void) {
  s32 offset;

  func_801C1ABC(2);
  offset = D_80181B11 * 0xA4;
  D_8014496F[offset] = D_8014496F[offset] & 0xFE;
  offset = D_80181B15 * 0xA4;
  D_8014496F[offset] = D_8014496F[offset] & 0xFE;
  offset = D_80181B16 * 0xA4;
  D_8014496F[offset] = D_8014496F[offset] & 0xFE;
  offset = D_80181B12 * 0xA4;
  D_8014496F[offset] = D_8014496F[offset] & 0xFE;
}
