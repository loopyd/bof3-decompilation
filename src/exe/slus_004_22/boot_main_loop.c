#include "internal.h"

/* @behavior Initializes the game runtime, then runs the permanent frame loop,
 * including the guarded EMI service/reset path.
 * @source 0x8014AAC8
 */
void boot_main_loop(void) {
  u8* work;

  boot_noop();
  D_8018B300 = 0;
  boot_init_runtime();
  boot_run_logo_exe();
  DrawSyncCallback(boot_capture_vsync);
  game_install_callback_slot(0, func_8014EA80);

  for (;;) {
    VSync(2);
    rand();
    PutDispEnv((DISPENV*)D_80143E68);
    PutDrawEnv((DRAWENV*)(D_80143E68 + 0x14));
    func_8014E22C();
    func_8014E6D0();
    DrawOTag((u_long*)(D_80143E68 + 0x8c));
    func_8014AFC0();
    func_8015D044();

    D_80143D44 ^= 1;
    work = D_80143D48 + D_80143D44 * 0x90;
    D_80143E68 = work;
    ClearOTagR((u_long*)(work + 0x70), 8);
    func_8014B020();

    if ((D_80145AA4 & 0x900) == 0x900) {
      if (D_80143F44 == 60) {
        if (emi_loader_is_ready() != 0) {
          D_80143F44 = 0;
          func_8015CEBC();
          game_dispatch_callback_slots();
          game_install_callback_slot(0, func_8014EA80);
        }
      } else {
        D_80143F44++;
      }
    } else {
      D_80143F44 = 0;
    }

    game_slot_scheduler_tick();
    func_80163010();
    D_80143EF8 = VSync(1);
    DrawSync(0);
    render_link_ot_packets();
    D_80143E6C++;
  }
}
