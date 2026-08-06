#include "internal.h"

/* @behavior clears bit 0 of the byte at offset 0x74 in the current work area.
 * @source 0x801AD69C
 */
void clearWorkAnimBit0_2(void) {
  struct GameWorkArea* work;

  work = GAME_WORK_AREA_PTR;
  *(u8*)&work->anim_state_74 &= 0xFE;
}
