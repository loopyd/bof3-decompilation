#include "bof3/ui/commu00_internal.h"

/* @source 0x801F23A4
 * @behavior Switches on the signed front-end progress byte: state 0 publishes
 * the selected-record index byte at 0x80145029 into the second byte of the
 * 0x801F2944 pair, scales that index by four and stops the shared selection
 * effect for it through func_8015D404 using the two leading bytes of the
 * 4-byte-stride EXE record pair at 0x80181EBA, queues effect group 10 (or 8
 * when the first byte of the 0x801F2944 pair is clear) through
 * func_801636A0 and advances the fairy progress byte; state 1 starts action
 * 0x2A1, latches the frontend mode byte to 2 and advances the fairy progress
 * byte once func_80163EA0 reports a nonzero low byte; state 2, unless the
 * frontend mode byte already reads 2, queues the frontend cue (published
 * index, 100, 8) through func_80161C20, clears the low three bits of the
 * shared front/world flag word, clears the UI selection state and dispatches
 * the current task variant resource; any other selector returns without side
 * effects.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F23A4(void) {
  u8* progress;

  progress = &D_801448EC;

  switch (*(s8*)progress) {
  case 0:
  {
    u32 value = D_80145029;

    D_801F2944[1] = value;
    value = value << 2;
    func_8015D404(D_80181EBA[value], D_80181EBA[value + 1u]);
    func_801636A0(D_801F2944[0] == 0 ? 8 : 10, 1);
    fairyProgress[0] += 1;
    break;
  }
  case 1:
    if ((u8)func_80163EA0() != 0) {
      func_80150224(0x2A1);
      D_80143BB0 = 2;
      *progress += 1;
    }
    break;
  case 2:
    if (D_80143BB0 != 2) {
      func_80161C20(D_801F2944[1], 100, 8);
      D_8014625A &= 0xFFF8u;
      clearUiSelectionState();
      dispatchCurrentTaskVariantResource();
    }
    break;
  }
}
