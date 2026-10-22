#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800A3F28
 * @behavior Raises bit 0x2000 of the battle global halfword D_801462E8, then
 *   marks flag 0x200 of the active battler's record (0x140-stride local-work
 *   record above 0x80145FB0 for index bytes below three, 0x118-stride
 *   enemy-work record above 0x801EB72C otherwise) and clears status bit 0 of
 *   the same record.
 * @status partial
 * @match 89.86
 * @residual 62/69 instructions, 272/276 bytes. The original encodes each of
 *   the three reads of the active battler index byte D_80146394 as its own
 *   `lui %hi; lbu %lo` materialization; this candidate shares one address
 *   register across the three reads, which is where the live diff stops. The
 *   non-volatile view of the first read is required: the battle15 internal
 *   header over-declares D_80146394 volatile, and the volatile spelling makes
 *   gcc truncate the loaded byte (a spurious `andi` that also stalls the load
 *   delay slot). The second read stays volatile so both branches re-read the
 *   cell as the original does. History: this candidate was requeued after a
 *   forbidden matching-aid removal; no aids remain (no register pins, clobbers,
 *   barriers or empty asm).
 */
void func_800A3F28(void) {
  volatile u16 *flags;
  u16 value;
  u32 index;

  flags = &D_801462E8;
  value = *flags;
  index = *(u8 *)&D_80146394;
  *flags = value | 0x2000;
  if (index < 3) {
    D_80145FB0[index].flags |= 0x200;
    D_80145FB0[*(volatile u8 *)&D_80146394].status &= 0xFE;
  } else {
    D_801EB72C[index - 3].flags |= 0x200;
    D_801EB72C[*(volatile u8 *)&D_80146394 - 3].status &= 0xFE;
  }
}
