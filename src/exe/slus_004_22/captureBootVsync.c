#include "internal.h"

/* @behavior captures the current vertical-blank counter for the boot frame.
 * @source 0x8014B17C
 */
void captureBootVsync(void) {
  D_80143EFC = VSync(1);
}
