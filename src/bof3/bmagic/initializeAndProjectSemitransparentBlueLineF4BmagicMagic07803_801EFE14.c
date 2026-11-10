#include "bof3/bof3.h"

/* @source 0x801EFE14
 * @behavior Initializes a LINE_F4, enables semitransparency and sets its RGB
 *           bytes to (0x40, 0x40, 0xC0). Projects four by-value SVECTORs into
 *           its four screen-coordinate pairs with RotAverage4 and stores the
 *           returned average depth through the second argument. The projection
 *           interpolation and flag outputs are local and discarded. The caller
 *           at 0x801EF7CC supplies the four eight-byte vectors in argument slots.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeAndProjectSemitransparentBlueLineF4BmagicMagic07803_801EFE14(
    LINE_F4 *line, long *depth, SVECTOR v0, SVECTOR v1, SVECTOR v2, SVECTOR v3) {
  long p;
  long flag;

  SetLineF4(line);
  SetSemiTrans(line, 1);
  line->r0 = 0x40;
  line->g0 = 0x40;
  line->b0 = 0xC0;
  *depth = RotAverage4(&v0, &v1, &v2, &v3,
                       (long *)&line->x0, (long *)&line->x1,
                       (long *)&line->x2, (long *)&line->x3, &p, &flag);
}
