#include "bof3/bof3.h"

/* Work record published through the scratchpad pointer cell at 0x1F800044. The
 * three reads the body needs come back as `lui`/`lw` pairs through this symbol,
 * while the macro spelling of the same cell folds the address into one base
 * register and diverges. */
/* @source 0x1F800044 @kind data */
extern u8* scratchpadWorkRecord;

/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* The local emitter at 0x801F4034 narrows both arguments with `sll`/`sra 16`
 * in its own body and the call site consumes no result. */
void func_801F4034(s16 arg0, s16 arg1);

/* @source 0x801F3898
 * @behavior Entry 3 of the four-entry work-byte-3 handler table based at
 *           0x801F5458, reached through the byte-3 dispatcher at 0x801F35FC.
 *           Adds 0x0A to work halfword 0x30 of the record published at the
 *           scratchpad pointer cell 0x1F800044, then compares the stored
 *           signed halfword against 0xF0 and clears state byte 0x03 once that
 *           height reaches it. Unless the shared mode byte 0x801454F2 is set,
 *           the byte at 0x80143BB0 is 2, or bit 0x100 of halfword 0x80146258
 *           is set, it then arms state byte 0x03 at 1 through a second read of
 *           the pointer cell. Finally it passes 0x5C and the signed work
 *           halfword 0x30 to the local emitter at 0x801F4034. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The 0xB0 bytes at payload offset 0xC98 are byte-identical to the exact
 * advanceHalfword30By0aAndSetState3To0AtF0OrTo1UnlessGatedThenDrawHalfword30World03Area12114_801F3940
 * span at payload offset 0xD40 in emi/world03/area121/14 apart from the single
 * `jal` word that encodes that overlay's own emitter address, whose target
 * shifts by exactly the delta between the two spans, and the same equality
 * holds against the exact World00Area03313_801F385C and
 * World02Area08713_801F395C spans at payload offsets 0xC5C and 0xD5C. This
 * body carries no other payload-relative word - every absolute address it
 * names is a shared cell outside the overlay - so the family statement order,
 * operand forms and the module-qualified name transfer whole, and the merged
 * span from 0xC98 onward is this target's own emitter.
 */
void advanceHalfword30By0aAndSetState3To0AtF0OrTo1UnlessGatedThenDrawHalfword30World03Area15113_801F3898(
    void) {
  u8* record;
  u8* emitter;
  s16* half;

  record = scratchpadWorkRecord;
  half = (s16*)(record + 0x30);
  *half = (s16)(*half + 0x0A);
  if (*half >= 0xF0) {
    record[3] = 0;
  }
  if (D_801454F2 == 0 && D_80143BB0 != 2 && (D_80146258 & 0x100u) == 0) {
    scratchpadWorkRecord[3] = 1;
  }
  emitter = scratchpadWorkRecord;
  func_801F4034(0x5C, FIELD_REF(s16, emitter, 0x30));
}
