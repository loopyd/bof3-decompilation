#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801DDD4C
 * @behavior Plays the cue of the newly selected AREA030 record value. While
 * the selected-record byte D_801E31F8 still differs from the last-cued byte
 * D_801E31FC, the same-binary sound-cue dispatcher func_8015DF18 receives the
 * cue 0x203 for value 0, 0x202 for value 1 and 0x205 for value 2; any other
 * value leaves the cue silent. The last-cued byte is not written here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DDD4C(void) {
  u8 cueState;

  cueState = D_801E31F8;
  if (cueState != D_801E31FC) {
    switch (cueState) {
    case 0:
      func_8015DF18(0x203);
      break;
    case 1:
      func_8015DF18(0x202);
      break;
    case 2:
      func_8015DF18(0x205);
      break;
    }
  }
}
