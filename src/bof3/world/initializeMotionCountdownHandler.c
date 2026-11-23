#include "bof3/world/area03004_internal.h"

/* @behavior Clear work bytes 8 and 6, seed byte 7 with 0x80 and countdown byte 9 with 4,
 * and clear bytes 0xA and 0xB. Clear position words 0x34, 0x38 and 0x3C and
 * motion words 0x0C and 0x10, store 0xFFFF at halfword 0x2C, and advance
 * handler byte 1.
 * @source 0x801D42E4
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeMotionCountdownHandler(void) {
  u32* workWords;
  u8* dispatchWork;
  u8 handlerIndex;

  D_1F800044[8u] = 0u;
  D_1F800044[6u] = 0u;
  D_1F800044[7u] = 128u;
  D_1F800044[9u] = 4u;
  D_1F800044[10u] = 0u;

  workWords = (u32*)D_1F800044;
  ((u8*)workWords)[11u] = 0u;
  workWords[13] = 0u;
  workWords[14] = 0u;
  workWords[15] = 0u;
  workWords[3] = 0u;
  workWords[4] = 0u;

  dispatchWork = (u8*)D_1F800044;
  handlerIndex = dispatchWork[1u];
  *(volatile u16*)(dispatchWork + 0x2Cu) = 0xFFFFu;
  dispatchWork[1u] = (u8)(handlerIndex + 1u);
}
