#include "bof3/scenario/scena16_internal.h"

/* @behavior finalizes the secondary SCENA16 path when the shared gate word
 * D_80143C40 reads zero: it clears the primary state byte D_80143C30 and the
 * effect bank D_8014832E, copies the pending palette block, queues front-end cues
 * 0x213 and 0x214, and, while the shared selection byte D_80145029 still names a
 * selection (not 0xFF), passes that byte's two-byte effect pair from the shared
 * table D_80181EBA to game_stop_selection_fx and marks the byte consumed with
 * 0xFF. It then sizes the front-end window through func_8016C0C0(0x7F, 0x7F) and
 * runs func_8014B8B0. The gate word is read before the frame is set up and the
 * selection byte is kept in $s0 across the two cue calls.
 * @source 0x801F7188
 * @status exact
 * @match 100.00
 * @residual none
 */
void finalizeSecondaryPath(void) {
  u8 selection;
  u8* slot;

  if (D_80143C40 == 0u) {
    D_80143C30 = 0u;
    D_8014832E = 0u;
    copyPaletteBlock();
    game_queue_frontend_cue(0x213u);
    game_queue_frontend_cue(0x214u);
    slot = &D_80145029;
    selection = *slot;

    if (selection != 0xffu) {
      u32 selection_offset;

      selection_offset = selection << 2;
      game_stop_selection_fx(
          (u32)D_80181EBA[selection_offset + 0u],
          (s32)D_80181EBA[selection_offset + 1u]);
      *slot = 0xffu;
    }

    func_8016C0C0(0x7f, 0x7f);
    func_8014B8B0();
  }
}
