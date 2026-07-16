#include "internal.h"

/* @behavior finalizes the secondary SCENA16 path and exits the local callback.
 * @source 0x801F7188
 */
void func_801F7188(void) {
  u32 selection;

  if (SCENA16_D_80143C40 == 0u) {
    SCENA16_D_80143C30 = 0u;
    SCENA16_D_8014832E = 0u;
    func_801F845C();
    game_queue_frontend_cue(0x213u);
    game_queue_frontend_cue(0x214u);
    selection = (u32)SCENA16_D_80145029;

    if (selection != 0xffu) {
      u32 selection_offset;

      selection_offset = selection << 2;
      game_stop_selection_fx(
          (u32)SCENA16_SELECTION_FX_TABLE[selection_offset + 0u],
          (s32)SCENA16_SELECTION_FX_TABLE[selection_offset + 1u]);
      SCENA16_D_80145029 = 0xffu;
    }

    func_8016C0C0(0x7f, 0x7f);
    func_8014B8B0();
  }
}
