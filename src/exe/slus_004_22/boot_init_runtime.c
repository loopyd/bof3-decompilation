#include "internal.h"

/* @behavior Initializes the runtime work buffers and subsystem state used by
 * the executable's boot loop.
 * @source 0x8014ACA0
 */
void boot_init_runtime(void) {
  u8* work;

  boot_init_disc_events();
  boot_init_display_envs();
  work = D_80143D48;
  boot_clear_ot_entry(work);
  boot_clear_ot_entry(work + 0x90);
  func_8014B6B4();
  D_80143D44 = 0;
  D_80143E68 = work;
  render_clear_rect(0, 0, 0x400, 0x200);
  DrawSync(0);
  SetDispMask(1);
  func_8014B020();
}
