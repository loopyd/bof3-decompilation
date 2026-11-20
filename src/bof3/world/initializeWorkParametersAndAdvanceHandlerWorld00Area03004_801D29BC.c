#include "bof3/world/area03004_internal.h"

/* @behavior Copy five shared bytes to work bytes 0x24 through 0x28, set byte 0x29 to 5,
 * clear byte 0x48, word 0x3C and byte 0x5C, and set bytes 0x5D through 0x5F to
 * -100. Clear words 0x60 through 0x6C, run the shared-parameter copy helper,
 * then advance handler byte 1.
 * @source 0x801D29BC
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeWorkParametersAndAdvanceHandlerWorld00Area03004(void) {
  u8* seedWork;
  u8* flagWork;
  u32* clearWords;
  u8* dispatchWork;

  D_1F800044[0x24u] = D_80143FEC[0u];
  D_1F800044[0x25u] = D_80143FEC[1u];
  D_1F800044[0x26u] = D_80143FEC[2u];
  D_1F800044[0x27u] = D_80143FEC[3u];
  D_1F800044[0x28u] = D_80143FEC[4u];
  D_1F800044[0x29u] = 5u;

  seedWork = (u8*)D_1F800044;
  seedWork[0x48u] = 0u;
  flagWork = (u8*)D_1F800044;
  *(volatile u32*)(seedWork + 0x3Cu) = 0u;
  flagWork[0x5Cu] = 0u;
  ((s8*)D_1F800044)[0x5Du] = -100;
  ((s8*)D_1F800044)[0x5Eu] = -100;
  ((s8*)D_1F800044)[0x5Fu] = -100;

  clearWords = (u32*)D_1F800044;
  clearWords[24] = 0u;
  clearWords[25] = 0u;
  clearWords[26] = 0u;
  clearWords[27] = 0u;

  copySharedWorkParametersOrClearWorkFlagsWorld00Area03004();

  dispatchWork = (u8*)D_1F800044;
  dispatchWork[1u] = (u8)(dispatchWork[1u] + 1u);
}
