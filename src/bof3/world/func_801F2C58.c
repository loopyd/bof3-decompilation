#include "bof3/world/area02414_internal.h"

/* @behavior overlay init entry (slot 0 of the entry table D_801F4200): seeds the
 * local 16-entry spin-work table, the local eight-entry work array and the
 * remaining local tables through func_801F3314, then clears scratch-work byte
 * 0x09 and increments the scratch work byte at 0x01.
 * @source 0x801F2C58
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 22/22 instructions and 88 -> 88 bytes.
 */
void func_801F2C58(void) {
  initSpinWork();
  func_801F3080();
  func_801F3314();
  D_1F800044[9] = 0;
  D_1F800044[1]++;
}
