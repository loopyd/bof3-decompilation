#include "internal.h"

/* @behavior captures the current vertical-blank counter for the boot frame.
 * @source 0x8014B17C
 */
void boot_capture_vsync(void) {
  D_80143EFC = VSync(1);
}
