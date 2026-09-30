#include "bof3/world/area01613_internal.h"

/* @source 0x801F30EC
 * @behavior refreshes the scratch byte at offset `0x07` from offset `0x0B`, then
 * when that byte is `1` searches the local seed record table at `0x801F4D7C`
 * for the record whose mask matches the shared seed word at `0x80143F10`, seeds
 * offsets `0x18`/`0x40`/`0x44`/`0x48`/`0x09` and calls the shared
 * `0x8014D6B8` helper with the record byte at offset `2`; scratch bytes `2`, `3`
 * and `4` seed the same offsets and call that helper with `3`, `0` and `1`, and
 * any other byte returns without a call; every calling path sets the mode byte
 * at offset `0x01` to `2`.
 * @status partial
 * @match 58.97
 * @residual first diff +0x0034: the 0x801F4D7C record load and the arm-1 store block split, and arm-1's 0x40/0x44/0x48/0x09 suffix is cross-jumped into the shared tail; the search loop also lacks the original's `move v1,a1` index copy.
 */
void func_801F30EC(void) {
  World00Area016Scratch* scratch;
  volatile World00Area016Scratch* target;
  World00Area016Scratch* reload;
  u32                     flag;
  u32                     state;
  u32                     value;

  scratch = D_1F800044;
  scratch->unk_04[3] = scratch->unk_0b;

  reload = D_1F800044;
  value = reload->unk_0b;
  if (value == 1) {
    u32 offset;
    u32 seed;

    seed = D_80143F10;
    offset = 0;
  search_seed:
    if (FIELD_REF(const u16, D_801F4D7C, offset) == seed) {
      goto seed_found;
    }
    offset += 4;
    goto search_seed;
  seed_found:
    target = D_1F800044;
    state = 2;
    target->unk_18 = seed;
    target->unk_40 = 0;
    target->unk_44 = 0x10000;
    target->unk_48 = state;
    D_1F800044->unk_04[5] = 8;
    flag = FIELD_REF(const u8, D_801F4D7C, offset + 2);
  } else {
    World00Area016Scratch* mode;
    u32                     state_byte;

    mode = D_1F800044;
    state_byte = mode->unk_0b;
    if (state_byte == 2) {
      flag = 3;
    } else if (state_byte == 3) {
      flag = 0;
    } else if (state_byte == 4) {
      flag = 1;
    } else {
      return;
    }

    target = D_1F800044;
    state = 2;
    target->unk_40 = 0;
    target->unk_44 = 0x10000;
    target->unk_48 = state;
    D_1F800044->unk_04[5] = 8;
  }

  func_8014D6B8(flag);
  D_1F800044->mode = state;
}
