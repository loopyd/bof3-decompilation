#include "bof3/battle/battle15_internal.h"

/* @behavior Initialises the primary battle selection-grid scratch band at
 * 0x80148570 from the active message-slot work record and the saved
 * group/page/cursor row of the 0x801454F4 table: the head byte, +0x04 and
 * +0x06 are written through the 0x80148570 base, the saved row through its own
 * symbols, and 0x80148584 receives bit 1 of the active selection slot flag word
 * when bit 1 of that word is set and bit 17 is clear.
 * @source 0x80097EC0
 * @status partial
 * @match 70.27
 * @residual 52/74 instructions at the original 296-byte size. The base is
 *   allocated to $t0 instead of $a3; flag/table loads and the final slot load
 *   also differ in scheduling and allocation. The original branch pair and
 *   its bit-mask delay slot match, and no stack frame remains.
 */
void func_80097EC0(void) {
  BattleSelectionState *state;
  u8  *active;
  u8   slot;
  u32  flags;
  u32  row_offset;

  state = &D_80148570;
  state->first = 1;
  D_80148571 = 8;
  D_80148572 = 2;
  D_80148573 = 2;
  slot = D_801EBF08[5];
  active = D_801EB4D8;
  flags = *(u32 *)(active + 0x10);
  D_8014857A = slot;
  /* Name the shared scale so folded table addresses need no orphan slots. */
  row_offset = slot * 3;
  D_8014857B = D_801454F4[row_offset];
  D_80148580 = D_801454F4[row_offset + 1];
  D_8014857D = 0xFF;
  D_80148578 = 2;
  D_80148579 = 0;
  D_8014857C = D_801454F4[row_offset + 2];

  if ((flags & 0x20002u) != 2u) {
    D_80148584 = 0;
  } else {
    D_80148584 = flags & 2u;
  }

  state->unk_04 = 0x140;
  state->unk_06 = 0x3F;
  D_801485DC = 0;
  D_801485DD = 8;
  D_801485DE = 0;
  D_80148656 = D_801EBF08[5];
}
