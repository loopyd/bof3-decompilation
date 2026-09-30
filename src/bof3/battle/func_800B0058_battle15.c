#include "bof3/battle/battle15_internal.h"

/* @source 0x800B0058
 * @behavior Returns the battle-participant lookup byte for the current
 *   participant index in D_80146374. Indices below three read the party
 *   record flag word at 0x80145FB8 + index*0x140: when flag bit 0x2 is set
 *   the byte comes from 0x800B6C74 indexed by D_801463C9, otherwise from
 *   0x800B6C68 indexed by the party byte 0x80145F09 + index*0x140. Larger
 *   indices read 0x800E40CF indexed by the 280-byte-stride enemy byte
 *   0x801EB710 + (index-3)*0x118.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_800B0058(void) {
  u32 index;
  u32 offset;

  /* Non-volatile view of the same byte: the original emits `lbu` straight into
   * the compared word register with no masking `andi`. */
  index = *(u8 *)&D_80146374;
  if (index < 3u) {
    offset = index * 0x140u;
    if ((D_80145FB8[index].flags_00 & 2u) != 0u) {
      return D_800B6C74[D_801463C9];
    }
    return D_800B6C68[D_80145F09[offset]];
  }
  index -= 3u;
  offset = D_801EB710[index * 0x118u];
  /* The class-table base stays an explicit fixed address: the original encodes
   * it as a full 32-bit constant (assembling to `addu at,index,at`), which a
   * declared symbol reference would not reproduce. */
  return PSX_REF(u8, 0x800E40CFu + offset * 0x88u);
}
