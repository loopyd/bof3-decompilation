#include "bof3/world/area00813_internal.h"

extern void func_8014D6B8(u32 flag);
extern void func_8015DF18(u16 cue);

/* @source 0x801F33F0
 * @behavior installs the entity selected by the scratch state entity index as
 *           the scratch work pointer, then selects mode 14 when the shared
 *           high-bit flag is set; when the shared secondary state byte
 *           0x80146865 reads 1 it arms that byte as 2, advances the shared
 *           selector byte 0x80146867, posts the shared cue 0x20D, resets the
 *           shared halfword counter 0x80146876 to 0xFF when the installed
 *           entity halfword 0x58 is below 5, requests the shared area resource
 *           selector with argument 2 and selects mode 7; when instead that
 *           entity reports halfword 0x58 equal to 8 and byte 0x4A equal to 1 it
 *           requests that selector with argument 4 and selects mode 4; finally
 *           restores the previous scratch work pointer.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 67/67 instructions, 268 bytes, live byte match.
 */
void func_801F33F0(void) {
  World00Area008State* state;
  World00Area008Entity* entity;
  u8* selector;
  u8 flags;
  u8 next;

  state = g_areaWork;
  entity = &D_80146888[state->entityIndex];
  selector = &D_80146867;
  flags = *selector;
  g_areaWork = (World00Area008State*)entity;

  if (flags & 0x80) {
    state->mode = 14;
  } else if (D_80146865 == 1) {
    next = flags + 1;
    D_80146865 = 2;
    *selector = next;
    func_8015DF18(0x20D);
    if (((World00Area008Entity*)g_areaWork)->unk_58 < 5) {
      D_80146876 = 0xFF;
    }
    func_8014D6B8(2);
    state->mode = 7;
  } else if (entity->unk_58 == 8 && entity->unk_4A == 1) {
    func_8014D6B8(4);
    state->mode = 4;
  }
  g_areaWork = state;
}
