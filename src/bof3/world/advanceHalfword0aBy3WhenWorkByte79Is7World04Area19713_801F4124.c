#include "bof3/bof3.h"

/* Family note: this 0x40-byte span is byte-identical to the span already
 * lifted exact at 0x801F3D38 of emi/world03/area143/13, proved by comparing
 * the two payloads at their own `runtime address - load address` offsets, so
 * the body below is that lift's statement form and both halves publish their
 * record through a global pointer cell outside this payload. The +0x79 byte of
 * the work record is the family's mode selector and 7 is the value whose arm
 * advances this overlay's halfword. */

extern u8* D_80146250;  /* @source 0x80146250 @kind unknown */
extern u16* D_80148210; /* @source 0x80148210 @kind unknown */

/* @source 0x801F4124
 * @behavior Overlay handler: reads the +0x79 byte of the work record published
 *           through the global pointer cell 0x80146250 and, when that byte
 *           equals 7, advances the halfword at offset 0x0A of the record
 *           published through the global pointer cell 0x80148210 by 3. Every
 *           other value of the byte returns before the halfword is touched. The
 *           halfword is read and written back through the one register that
 *           holds the 0x80148210 record. Takes no arguments, returns nothing,
 *           calls nothing and reserves no stack frame. No jal immediate in this
 *           payload names 0x801F4124, and the only payload word equal to its
 *           address is an entry of the handler pointer run at 0x801F49BC, so
 *           the body is installed as a callback by an outside caller and its
 *           parameter roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword0aBy3WhenWorkByte79Is7World04Area19713_801F4124(void) {
  if (D_80146250[0x79] == 7) {
    D_80148210[5] += 3;
  }
}
