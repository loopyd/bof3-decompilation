#include "bof3/ui/game00_internal.h"

/* @behavior clears three consecutive words at offsets 0x0c through 0x14 in
 * the work record referenced by scratchpad pointer slot 17.
 * @source 0x801C5798
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void clearWorkWordsCTo14(void) {
  struct GameWorkArea** base;
  struct GameWorkArea*  temp_v0;

  base = (struct GameWorkArea**)0x1F800000;
  temp_v0 = base[0x11];
  ((s32*)temp_v0)[3] = 0;
  ((s32*)temp_v0)[4] = 0;
  ((s32*)temp_v0)[5] = 0;
}
