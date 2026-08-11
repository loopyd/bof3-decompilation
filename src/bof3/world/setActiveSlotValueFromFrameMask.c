#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801DB0AC
 * @behavior Clears or invalidates the active slot state according to a frame-mask bit.
 * @status matching
 */
void setActiveSlotValueFromFrameMask(void) {
  if ((D_80143E6C & (3 >> ((u8*)D_801E320C)[3])) == 0) {
    D_801468A4[D_801E3208].value = -1;
  } else {
    D_801468A4[D_801E3208].value = 0;
  }
}
