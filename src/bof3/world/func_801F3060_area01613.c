#include "bof3/world/area01613_internal.h"

/* @source 0x801F3060
 * @behavior resets the shared area scratch record: writes 0x80 to offset 0x24,
 * calls func_8014DD3C with 0x0F, then writes 5 to offset 0x29, clears offsets
 * 0x2A and 0x5D..0x5F, and sets the mode byte at offset 0x01 to 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3060(void) {
  FIELD_REF(u8, D_1F800044, 0x24u) = 0x80;
  func_8014DD3C(0x0F);
  FIELD_REF(u8, D_1F800044, 0x29u) = 5;
  FIELD_REF(u8, D_1F800044, 0x2Au) = 0;
  FIELD_REF(u8, D_1F800044, 0x5Du) = 0;
  FIELD_REF(u8, D_1F800044, 0x5Eu) = 0;
  FIELD_REF(u8, D_1F800044, 0x5Fu) = 0;
  FIELD_REF(u8, D_1F800044, 0x01u) = 1;
}
