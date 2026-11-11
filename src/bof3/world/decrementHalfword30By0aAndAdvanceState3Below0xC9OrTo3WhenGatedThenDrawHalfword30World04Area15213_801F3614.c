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

/* @source 0x801F3614
 * @behavior Byte-3 state handler 1 of this overlay's handler run based at
 *           0x801F4D84, whose four words are 0x801F3594, 0x801F3614,
 *           0x801F36EC and 0x801F37EC. Subtracts 0x0A from the signed halfword
 *           at offset 0x30 of the work record published at the scratchpad
 *           pointer cell 0x1F800044 and stores the truncated halfword, then
 *           increments work byte 0x03 once that stored value falls below
 *           0xC9. When the shared byte 0x801454F2 is nonzero and work byte 0x0B
 *           is nonzero, the byte at 0x80143BB0 is 2, or bit 0x100 of the
 *           halfword at 0x80146258 is set, it then writes 3 to work byte 0x03
 *           through a second read of the pointer cell. Finally it reloads that
 *           cell once more and passes mode 0x5C and the signed work halfword
 *           0x30 to the overlay drawer 0x801F3F88, discarding any result. It
 *           takes no arguments, returns nothing and saves only the return
 *           address at 0x10 of its 0x18-byte frame. The 0xD8 bytes match
 *           decrementHalfword30By0aAndAdvanceState3Below0xC9OrTo3WhenGatedThenDrawHalfword30World03Area12114_801F3768
 *           apart from the single `jal` word at offset 0xC0, which carries each
 *           overlay's own drawer address; the two payload spans compare equal
 *           at every other byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By0aAndAdvanceState3Below0xC9OrTo3WhenGatedThenDrawHalfword30World04Area15213_801F3614(
    void) {
  u8* record;
  u8* emitter;
  s16* half;

  record = area15213WorkCursor_1F800044;
  half = (s16*)(record + 0x30);
  *half = (s16)(*half - 0x0A);
  if (*half < 0xC9) {
    record[3]++;
  }
  if ((D_801454F2 != 0 && area15213WorkCursor_1F800044[0x0B] != 0) ||
      D_80143BB0 == 2 || (D_80146258 & 0x100u) != 0) {
    area15213WorkCursor_1F800044[3] = 3;
  }
  emitter = area15213WorkCursor_1F800044;
  func_801F3F88(0x5C, FIELD_REF(s16, emitter, 0x30));
}
