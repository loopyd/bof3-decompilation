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

/* @source 0x801F385C
 * @behavior Entry 9 of the ten-entry work-byte-1 handler table based at
 *           0x801F4F78 in this overlay, entry 7 of the work-byte-2 table based
 *           at 0x801F4F80 and entry 3 of the four-entry work-byte-3 run based
 *           at 0x801F4F90 — one pointer word at 0x801F4F9C read from all three
 *           bases — for the work record published at the scratchpad pointer
 *           cell 0x1F800044. Adds 0x0A to the halfword at record offset 0x30,
 *           stores the truncated halfword, and clears work state byte 0x03 once
 *           that stored signed value reaches 0xF0. Unless the shared mode byte
 *           0x801454F2 is set, byte 0x80143BB0 reads 2, or bit 0x100 of halfword
 *           0x80146258 is set, it then writes 1 to work byte 0x03 through a
 *           second read of the pointer cell. Finally it reads that cell once
 *           more and passes 0x5C and the signed work halfword 0x30 to the
 *           overlay emitter 0x801F3FF8, discarding any result. No in-image `jal`
 *           names this address; only that one pointer word outside the three
 *           table views does. It takes no arguments, uses no result and saves
 *           only the return address in its 0x18-byte frame.
 *           The 0xB0 bytes are byte-identical to the span at 0x801F3940 in
 *           emi/world03/area121/14 apart from the single internal `jal` word,
 *           whose bias equals the 0xE4 offset between the two spans, so this
 *           body has taken that sibling's statement order, operand forms and
 *           offset spelling while its own literals, table memberships and
 *           callee follow this overlay's bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateAdvanceHalfwordReset(
    void) {
  u8* record;
  u8* emitter;
  s16* half;

  record = D_1F800044;
  half = (s16*)(record + 0x30);
  *half = (s16)(*half + 0x0A);
  if (*half >= 0xF0) {
    record[3] = 0;
  }
  if (D_801454F2 == 0 && D_80143BB0 != 2 && (D_80146258 & 0x100u) == 0) {
    D_1F800044[3] = 1;
  }
  emitter = D_1F800044;
  func_801F3FF8(0x5C, FIELD_REF(s16, emitter, 0x30));
}
