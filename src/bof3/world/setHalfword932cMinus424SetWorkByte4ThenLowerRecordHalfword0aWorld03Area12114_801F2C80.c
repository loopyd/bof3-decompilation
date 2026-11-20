#include "bof3/bof3.h"

extern s16 D_8014932C;
extern u16* D_80148210;

/* @source 0x801F2C80
 * @behavior Overlay setter and step: stores the signed halfword -0x424
 * (0xFBDC) into the shared halfword at 0x8014932C, then marks the scratch
 * work record published through the 0x1F800044 pointer cell by writing 1 into
 * its byte 4, and finally subtracts 2 from the halfword at offset 0x0A of the
 * record published through the global pointer cell 0x80148210. The two stores
 * come before the record load and the halfword update lands in the return
 * delay slot. No word of this payload calls or points at the entry, so the
 * argument and return roles stay structural; it builds no frame and calls
 * nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setHalfword932cMinus424SetWorkByte4ThenLowerRecordHalfword0aWorld03Area12114_801F2C80(
    void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  D_8014932C = -0x424;
  work[4] = 1;
  D_80148210[5] -= 2;
}
