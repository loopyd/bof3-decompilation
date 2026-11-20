#include "bof3/world/area03004_internal.h"

/* @behavior Call applyMainLookupRecord(0x2B), initialize the panel fields and call
 * selectActiveWorkSequence with work byte 6. Request the selected halfword cue
 * unless it is 0xFFFF, then advance handler byte 1.
 * @source 0x801D339C
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializePanelWorkRequestSelectedCueAndAdvanceHandlerWorld00Area03004(void) {
  u8* geometryWork;
  u8* clearWork;
  u8* setupWork;
  u8* dispatchWork;
  u8* selectorWork;
  u16 cue;

  applyMainLookupRecord(43);

  geometryWork = (u8*)D_1F800044;
  *(volatile s16*)(geometryWork + 0x2Eu) = 390;
  geometryWork[0x24u] = 128u;
  clearWork = (u8*)D_1F800044;
  *(volatile s16*)(geometryWork + 0x30u) = 110;
  clearWork[0x48u] = 0u;
  setupWork = (u8*)D_1F800044;
  setupWork[0x29u] = 2u;
  ((u8*)D_1F800044)[0x5Du] = 0u;
  ((u8*)D_1F800044)[0x5Eu] = 0u;
  ((u8*)D_1F800044)[0x5Fu] = 0u;
  ((u8*)D_1F800044)[0x2Au] = 0u;

  selectorWork = (u8*)D_1F800044;
  selectActiveWorkSequence(selectorWork[6u]);

  cue = D_801E1DA0[((u8*)D_1F800044)[6u]];
  if (cue != 0xFFFFu) {
    func_8015DF18(cue);
  }

  dispatchWork = (u8*)D_1F800044;
  dispatchWork[1u] = (u8)(dispatchWork[1u] + 1u);
}
