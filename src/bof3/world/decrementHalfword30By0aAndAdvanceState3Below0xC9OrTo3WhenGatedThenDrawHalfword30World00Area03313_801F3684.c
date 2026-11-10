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

/* @source 0x801F3684
 * @behavior Entry 12 of this overlay's fifteen-entry handler run 0x801F4F64
 *           through 0x801F4F9C: entry 7 of the work-byte-1 table based at
 *           0x801F4F78, entry 5 of the work-byte-2 table based at 0x801F4F80 and
 *           entry 1 of the four-entry work-byte-3 run based at 0x801F4F90, for
 *           the work record published at the scratchpad pointer cell 0x1F800044.
 *           Subtracts 0x0A from the halfword at record offset 0x30 and stores
 *           the truncated halfword, then increments work state byte 0x03 once
 *           that stored signed value falls below 0xC9. When the shared mode byte
 *           0x801454F2 is nonzero and work byte 0x0B is nonzero, or byte
 *           0x80143BB0 is 2, or bit 0x100 of halfword 0x80146258 is set, it
 *           writes 3 to work byte 0x03 through a second read of the pointer
 *           cell. Finally it reads that cell once more and passes 0x5C and the
 *           signed work halfword 0x30 to the overlay emitter 0x801F3FF8,
 *           discarding any result. No in-image `jal` or data word outside those
 *           three table views names this address. It takes no arguments, uses no
 *           result and saves only the return address in its 0x18-byte frame.
 *           The 0xD8 bytes matched the reviewed
 *           decrementHalfword30By0aAndAdvanceState3Below0xC9OrTo3WhenGatedThenDrawHalfword30World03Area12114_801F3768
 *           in emi/world03/area121/14 apart from the single internal `jal` word,
 *           whose bias equals the 0xE4 offset between the two spans, so this
 *           body takes that sibling's statement order while the pointer-cell
 *           spelling follows this overlay's 0x801F353C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By0aAndAdvanceState3Below0xC9OrTo3WhenGatedThenDrawHalfword30World00Area03313_801F3684(
    void) {
  u8* record;
  u8* draw;
  s16* half;

  record = D_1F800044;
  half = (s16*)(record + 0x30);
  *half = (s16)(*half - 0x0A);
  if (*half < 0xC9) {
    record[3]++;
  }
  if ((D_801454F2 != 0 && D_1F800044[0x0B] != 0) || D_80143BB0 == 2 ||
      (D_80146258 & 0x100u) != 0) {
    D_1F800044[3] = 3;
  }
  draw = D_1F800044;
  func_801F3FF8(0x5C, FIELD_REF(s16, draw, 0x30));
}
