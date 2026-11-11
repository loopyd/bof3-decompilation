#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area15213WorkCursor_1F800044;
/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* Both call sites in this overlay pass the mode 0x5C and the signed work
 * halfword 0x30; the drawer's own prologue narrows two arguments with
 * `sll`/`sra 16` and no call site consumes a result. */
void func_801F3F88(s16 arg0, s16 arg1);

/* @source 0x801F36EC
 * @behavior Byte-3 state handler 2 of this overlay's handler run based at
 *           0x801F4D84, whose four words are 0x801F3594, 0x801F3614,
 *           0x801F36EC and 0x801F37EC. While work byte 0x0B of the work record
 *           published at scratchpad pointer cell 0x1F800044 is still zero it
 *           increments work byte 0x09 and, once that stored byte reaches 0x5A,
 *           increments work byte 0x0B through a second read of the pointer
 *           cell. It then increments work byte 0x03 through another read of
 *           that cell when the shared byte 0x801454F2 and work byte 0x0B are
 *           both nonzero, or when the byte at 0x80143BB0 is 2, or when bit
 *           0x100 of the halfword at 0x80146258 is set. Finally it reloads the
 *           cell once more and passes mode 0x5C and the signed work halfword
 *           0x30 to the overlay drawer 0x801F3F88, discarding any result. It
 *           takes no arguments, returns nothing and saves only the return
 *           address at 0x10 of its 0x18-byte frame. The 0x100-byte payload
 *           span equals the one at
 *           advanceWorkByte9To0x5aAndIncrementByteBThenAdvanceState3WhenGatedThenDrawHalfword30World01Area06513_801F3860
 *           and the one at
 *           advanceWorkByte9To0x5aAndIncrementByteBThenAdvanceState3WhenGatedThenDrawHalfword30World03Area11513_801F382C
 *           at every byte except the two low bytes of the `jal` word at offset
 *           0xE8, which encode each overlay's own drawer address; one payload
 *           `cmp` per sibling put those 0xFE remaining bytes equal, so this
 *           body takes that family's statement order with this target's
 *           drawer.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte9To0x5aAndIncrementByteBThenAdvanceState3WhenGatedThenDrawHalfword30World04Area15213_801F36EC(
    void) {
  u8* record;
  u8* emitter;

  record = area15213WorkCursor_1F800044;
  if (record[0x0B] == 0) {
    record[9]++;
    record = area15213WorkCursor_1F800044;
    if (record[9] >= 0x5A) {
      record[0x0B]++;
    }
  }
  if ((D_801454F2 != 0 && area15213WorkCursor_1F800044[0x0B] != 0) ||
      D_80143BB0 == 2 || (D_80146258 & 0x100u) != 0) {
    area15213WorkCursor_1F800044[3]++;
  }
  emitter = area15213WorkCursor_1F800044;
  func_801F3F88(0x5C, FIELD_REF(s16, emitter, 0x30));
}
