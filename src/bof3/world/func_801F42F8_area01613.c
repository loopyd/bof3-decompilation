#include "bof3/world/area01613_internal.h"

/* @source 0x801F42F8
 * @behavior Calls the shared `func_8014DD3C` helper with `0x46`, clears the
 * scratch bytes at `0x48` and `0x24` and writes `5` to offset `0x29`; copies the
 * signed pair selected by the scratch step index at offset `0x08` from the local
 * table `0x801F51B8` into the scratch words at `0x0C` and `0x10` and reloads the
 * scratch words at `0x34`/`0x38` from `0x80145EC4`/`0x80145EC8`; while the
 * scratch byte at `0x06` is set it subtracts the top-three-bit signed field of
 * that pair from the scratch halves at `0x36`/`0x3A` and nudges one half by `2`
 * when the copied word at `0x0C` is clear. It then subtracts the top-seven-bit
 * signed field of the same pair from those halves, clears the scratch byte at
 * `0x0B`, increments the mode byte at `0x01`, calls the shared `func_8014D6B8`
 * helper with the first byte of the local pair at `0x801F51C8` selected by the
 * step index, stores that pair's second byte to scratch offset `0x2A` and
 * finally calls the shared `func_8014D290` helper.
 * @status partial
 * @match 65.73
 * @residual 94/143 instructions; 560 original bytes versus 572 current.
 * The context loads move ahead of scratch stores, the branch keeps an extra
 * byte mask, and store scheduling differs. The volatile pointer-cell reads
 * preserve the original repeated work-pointer loads.
 */
void func_801F42F8(void) {
  s16 stepped;

  func_8014DD3C(0x46);

  D_1F800044->unk_48 = 0;
  FIELD_REF(u8, D_1F800044, 0x24u) = 0;
  FIELD_REF(u8, D_1F800044, 0x29u) = 5;

  D_1F800044->unk_0c = (s16)D_801F51B8[D_1F800044->unk_04[4]][0];
  D_1F800044->unk_10 = (s16)D_801F51B8[D_1F800044->unk_04[4]][1];
  D_1F800044->unk_34 = D_80145EC4;
  D_1F800044->unk_38 = D_80145EC8;

  if (D_1F800044->unk_04[2] != 0) {
    FIELD_REF(u16, D_1F800044, 0x36u) -=
        (s16)D_801F51B8[D_1F800044->unk_04[4]][0] >> 13;
    stepped = FIELD_REF(u16, D_1F800044, 0x3au) -
              ((s16)D_801F51B8[D_1F800044->unk_04[4]][1] >> 13);
    FIELD_REF(u16, D_1F800044, 0x3au) = stepped;

    if (D_1F800044->unk_04[2] == 1) {
      if (D_1F800044->unk_0c == 0) {
        FIELD_REF(u16, D_1F800044, 0x36u) += 2;
      } else {
        FIELD_REF(u16, D_1F800044, 0x3au) = stepped + 2;
      }
    } else {
      if (D_1F800044->unk_0c == 0) {
        FIELD_REF(u16, D_1F800044, 0x36u) -= 2;
      } else {
        FIELD_REF(u16, D_1F800044, 0x3au) = stepped - 2;
      }
    }
  }

  FIELD_REF(u16, D_1F800044, 0x36u) -= (s16)D_801F51B8[D_1F800044->unk_04[4]][0] >> 9;
  FIELD_REF(u16, D_1F800044, 0x3au) -= (s16)D_801F51B8[D_1F800044->unk_04[4]][1] >> 9;
  D_1F800044->unk_0b = 0;
  D_1F800044->mode = D_1F800044->mode + 1;
  func_8014D6B8(D_801F51C8[D_1F800044->unk_04[4]][0]);
  FIELD_REF(u8, D_1F800044, 0x2au) = D_801F51C8[D_1F800044->unk_04[4]][1];
  func_8014D290();
}
