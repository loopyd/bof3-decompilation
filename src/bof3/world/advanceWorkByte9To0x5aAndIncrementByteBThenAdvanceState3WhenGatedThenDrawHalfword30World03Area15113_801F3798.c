#include "bof3/bof3.h"

extern u8 D_801454F2;
extern u8 D_80143BB0;
extern u16 D_80146258;
/* Work record published through the scratchpad pointer cell at 0x1F800044. The
 * two reads the body needs come back as `lui`/`lw` pairs through this symbol,
 * while the macro spelling of the same cell folds the address into one base
 * register and diverges. */
/* @source 0x1F800044 @kind data */
extern u8* D_1F800044;

/* The local emitter at 0x801F4034 narrows both arguments with `sll`/`sra 16`
 * in its own body and the call site consumes no result. */
void func_801F4034(s16 arg0, s16 arg1);

/* @source 0x801F3798
 * @behavior Entry 2 of the handler table based at 0x801F5458, which the byte-3
 *           dispatcher at 0x801F35FC indexes with record byte 0x03 of the work
 *           record published at the scratchpad pointer cell 0x1F800044. While
 *           record byte 0x0B is still zero it increments record byte 0x09 and,
 *           once that stored byte reaches 0x5A, increments record byte 0x0B
 *           through a second read of the pointer cell. It then increments state
 *           byte 0x03 through another read of the cell when the shared byte
 *           0x801454F2 is set and record byte 0x0B is nonzero, or the phase byte
 *           0x80143BB0 is 2, or bit 0x100 of halfword 0x80146258 is set. Finally
 *           it reads the cell again and passes 0x5C and the signed halfword at
 *           0x30 to the local emitter at 0x801F4034. Takes no arguments and
 *           returns nothing. The 0x100 bytes were byte-identical to the exact
 *           advanceWorkByte9To0x5aAndIncrementByteBThenAdvanceState3WhenGatedThenDrawHalfword30World03Area11513_801F382C
 *           in emi/world03/area115/13 apart from the single `jal` word that
 *           encodes that overlay's own emitter address, whose target shifts by
 *           exactly the offset between the two spans, so this body follows the
 *           reviewed sibling statement order with this target emitter.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte9To0x5aAndIncrementByteBThenAdvanceState3WhenGatedThenDrawHalfword30World03Area15113_801F3798(
    void) {
  u8* record;
  u8* emitter;

  record = D_1F800044;
  if (record[0x0B] == 0) {
    record[9]++;
    record = D_1F800044;
    if (record[9] >= 0x5A) {
      record[0x0B]++;
    }
  }
  if ((D_801454F2 != 0 && D_1F800044[0x0B] != 0) || D_80143BB0 == 2 ||
      (D_80146258 & 0x100u) != 0) {
    D_1F800044[3]++;
  }
  emitter = D_1F800044;
  func_801F4034(0x5C, FIELD_REF(s16, emitter, 0x30));
}
