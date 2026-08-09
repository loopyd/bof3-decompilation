#include "bof3/core/slus_internal.h"

/* @behavior captures the current vertical-blank counter for the boot frame.
 * @source 0x8014B17C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void captureBootVsync(void) {
  D_80143EFC = VSync(1);
}
