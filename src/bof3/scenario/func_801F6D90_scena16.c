#include "bof3/scenario/scena16_internal.h"

/* @behavior routes the primary SCENA16 area path from the pending world id
 * D_80143F00, read with an unsigned halfword load before the frame is set up:
 * world 2 runs func_801F6F30 and world 4 runs seedRouteEnterState2, both leaving
 * D_80143C30 = 1; world 0x1F runs seedRouteEnterState3 and leaves D_80143C30 = 0.
 * Every other world id keeps D_80143C30 unchanged. All paths then store 2 in the
 * secondary route byte D_80146872, which is also the value the two shared
 * `D_80143C30 = 1` arms carry into that store through the shared exit block.
 * @source 0x801F6D90
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F6D90(void) {
  switch (D_80143F00) {
    case 2:
      func_801F6F30();
      D_80143C30 = 1u;
      break;

    case 4:
      seedRouteEnterState2();
      D_80143C30 = 1u;
      break;

    case 0x1f:
      seedRouteEnterState3();
      D_80143C30 = 0u;
      break;
  }

  D_80146872 = 2;
}
