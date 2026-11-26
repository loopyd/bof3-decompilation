#include "bof3/bof3.h"

extern u8 D_801454F2;
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void drawMarkerSpriteLayers(
    s16 arg0, s16 arg1);

/* @source 0x801F3578
 * @behavior Entry 3 of the four-entry work-byte-2 handler table based at
 *           0x801F5448, reached through the dispatcher
 *           dispatchByte2Table5448. Reads the
 *           work record through the scratchpad pointer cell 0x1F800044, stores
 *           that record's halfword 0x2E less 0x10, and clears the work state
 *           byte 0x02 when the stored signed value is below -0x2F. It then
 *           writes 1 to state byte 0x02 through a second lookup of the pointer
 *           cell unless the shared byte 0x801454F2 reads 2, and finally draws
 *           the marker sprite layers at x 0x10 and y equal to the signed
 *           halfword 0x2E of a third lookup of that cell. Takes no arguments,
 *           uses no result, and its 0x18-byte frame holds the return address
 *           only.
 *           The tail from the mode-byte test on follows the statement shape of
 *           the exact
 *           setWorkState2To3
 *           body with this entry's inverted arm: one cmp of the 0x40-byte tails
 *           at payload offsets 0x9B0 and 0x92C found them equal apart from the
 *           branch opcode, the stored state constant (1 here, 3 there) and the
 *           slot each body puts its return-address store in. Both load the
 *           drawer argument with `lh` from record offset 0x2E.
 *           The halfword update takes the store-then-extend form the reviewed
 *           countdown handlers record for a stored value that is tested rather
 *           than reloaded. Each of its three cell lookups is a named pointer
 *           cell, so every read materialises its own `lui`/`lw` pair and the
 *           first sits above the frame setup.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatResetIsDraw(
    void) {
  u8* record;
  u8* cursor;
  u16 half;

  record = D_1F800044;
  half = FIELD_REF(u16, record, 0x2E) - 0x10;
  FIELD_REF(u16, record, 0x2E) = half;
  if ((s16)half < -0x2F) {
    record[2] = 0;
  }
  if (D_801454F2 != 2) {
    D_1F800044[2] = 1;
  }
  cursor = D_1F800044;
  drawMarkerSpriteLayers(
      0x10, FIELD_REF(s16, cursor, 0x2E));
}
