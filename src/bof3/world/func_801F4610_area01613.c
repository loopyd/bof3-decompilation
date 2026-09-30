#include "bof3/world/area01613_internal.h"

/* @source 0x801F4610
 * @behavior When the shared byte at `0x8014496D` is `9`, or bit `0` of the
 * shared byte at `0x80146871` is set, calls the shared `0x80196070` helper and
 * returns; otherwise calls the shared `0x8019A194` helper, requests resource
 * `0x205` through the shared `func_8014DD3C` helper, clears the scratch bytes at
 * offsets `0x48`, `0x24`, `0x2A` and `0x5D`..`0x5F`, then stamps `0xA0` into the
 * map byte at the column and row held by the four-byte record at `0x801F4DA0`
 * selected by the scratch byte at offset `0x0B` (`0x8014931C + row *
 * 0x80104000 + column`, the map base and width used by `func_80166CB0`), calls
 * the shared `func_8014D6B8` helper with `0` and finally sets the mode byte at
 * offset `0x01` to `1`.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F4610(void) {
  u8 index;

  if (D_8014496D == 9 || (D_80146871 & 1) != 0) {
    func_80196070();
    return;
  }

  func_8019A194();
  func_8014DD3C(0x205);
  D_1F800044->unk_48 = 0;
  FIELD_REF(u8, D_1F800044, 0x24u) = 0;
  FIELD_REF(u8, D_1F800044, 0x2Au) = 0;
  FIELD_REF(u8, D_1F800044, 0x5Du) = 0;
  FIELD_REF(u8, D_1F800044, 0x5Eu) = 0;
  FIELD_REF(u8, D_1F800044, 0x5Fu) = 0;

  index = D_1F800044->unk_0b;
  *(u8*)(D_8014931C + D_801F4DA0[index][1] * D_80104000 +
         D_801F4DA0[index][0]) = 0xA0;

  func_8014D6B8(0);
  FIELD_REF(u8, D_1F800044, 0x01u) = 1;
}
