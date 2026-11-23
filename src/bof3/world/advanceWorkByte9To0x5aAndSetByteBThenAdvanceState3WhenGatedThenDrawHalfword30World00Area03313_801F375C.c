#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* D_1F800044;
/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* The overlay emitter narrows both arguments with `sll`/`sra 16` in its own
 * prologue and this call site consumes no result. */
void func_801F3FF8(s16 arg0, s16 arg1);

/* @source 0x801F375C
 * @behavior Entry 8 of the ten-entry work-byte-1 handler table based at
 *           0x801F4F78 in this overlay, entry 6 of the work-byte-2 table based
 *           at 0x801F4F80 and entry 2 of the four-entry work-byte-3 run based
 *           at 0x801F4F90 — one pointer word at 0x801F4F98 read from all three
 *           bases — for the work record published at the scratchpad pointer
 *           cell 0x1F800044. While work byte 0x0B is still zero it increments
 *           work byte 0x09 and, once that byte re-read through the pointer cell
 *           reaches 0x5A, increments work byte 0x0B as well. When the shared mode
 *           byte 0x801454F2 is nonzero and work byte 0x0B is nonzero, byte
 *           0x80143BB0 reads 2, or bit 0x100 of halfword 0x80146258 is set, it
 *           then increments work byte 0x03 through a further read of the pointer
 *           cell. Finally it reads that cell once more and passes 0x5C and the
 *           signed work halfword 0x30 to the overlay emitter 0x801F3FF8,
 *           discarding any result. No in-image `jal` names this address; only
 *           that one pointer word outside the three table views does. It takes
 *           no arguments, uses no result and saves only the return address in
 *           its 0x18-byte frame.
 *           The 0x100 bytes are byte-identical to the span at 0x801F3840 in
 *           emi/world03/area121/14 apart from the single internal `jal` word,
 *           whose bias equals the 0xE4 offset between the two spans, so this
 *           body takes that sibling's statement order, operand forms and offset
 *           spelling while its own literals, table memberships and callee
 *           follow this overlay's bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte9To0x5aAndSetByteBThenAdvanceState3WhenGatedThenDrawHalfword30World00Area03313_801F375C(
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
  func_801F3FF8(0x5C, FIELD_REF(s16, emitter, 0x30));
}
