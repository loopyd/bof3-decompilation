#include "bof3/bof3.h"

/* Front-end phase byte of the shared main executable, outside this payload. The
 * neighbouring step 0x801F3CE4 of this overlay tests the same byte for the same
 * value 5, and the sister overlay world03/area144 tests the counterpart byte
 * 0x80146189 against 4 in the same position, so the byte keeps the address-
 * anchored name the target has no map row for.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Record published through the global pointer cell at 0x80148210, outside this
 * payload. The original reads and writes its halfword 0x0A through one loaded
 * address, so the cell is declared at halfword width; the sibling handler
 * 0x801F4D90 of this same overlay lowers the same halfword by 2 in the same
 * form.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;

/* @source 0x801F3D40
 * @behavior State handler of this overlay, reached through the word 0x801F6610,
 *           entry 2 of the dispatch table that publishes this payload's
 *           handlers, so no jal names it. When the shared front-end phase byte
 *           at 0x80143BB0 reads 5 it lowers the halfword at offset 0x0A of the
 *           work record published through the global pointer cell 0x80148210 by
 *           2; any other value leaves the record untouched. The local handler
 *           0x801F3CE4 of the same table tests that byte for the same value on
 *           its untaken arm, and the remaining arm direction is this body's
 *           whole effect. Takes no arguments, returns nothing and uses no stack
 *           frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword0SharedByte5(void) {
  if (D_80143BB0 == 5) {
    D_80148210[5] -= 2;
  }
}
