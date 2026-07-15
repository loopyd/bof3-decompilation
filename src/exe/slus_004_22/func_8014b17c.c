#include "internal.h"

/* @behavior captures the current vertical-blank counter for the boot frame.
 * @source 0x8014b17c func_8014b17c
 */
void func_8014b17c(void) {
  D_80143EFC = VSync(1);
}
