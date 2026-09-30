#include "bof3/battle/battle03_internal.h"

/* @source 0x801E99AC
 * @behavior Selects the 0x118-byte record D_801EB630[panel byte +0x0A - 3]:
 * stores panel halfword +0x04 as that record's signed halfword +0x2E plus its
 * signed byte +0xDA, adding the module byte 0x801EAEA4 while the record byte
 * +0x08 is 0 or 3 and 0x801EAEA6 otherwise; then stores panel halfword +0x06 as
 * the record's unsigned halfword +0x30 plus its signed byte +0xDB plus 0xC and
 * forwards panel halfwords +0x04/+0x06 with panel byte +0x0A to func_801D8690.
 * Finally, while the selection gate byte 0x801462EF is set, a queued slot byte
 * D_801EB4D8 that has no 0xC0 bits and equals panel byte +0x0A, or that has
 * 0xC0 bits and 0x40 set, skips the queued panel task commit.
 * @status partial
 * @match 97.80
 * @residual live asm diff 89/91 instructions, 364 versus 364 bytes. First
 * mismatch +0x0090: this build re-materializes the record index mask
 * (andi v1,a3,0xff) before the panel halfword +0x04 store, the original stores
 * first. Second residual: the func_801D8690 first argument is read through the
 * retained pointer register instead of the reloaded one (lh a0,4(a2) here versus
 * lh a0,4(v0) there). Both are register-allocation/scheduling only: no missing
 * symbol, type, boundary or control-flow fact.
 * Matching note: panel halfword +0x04 is the first use of the D_80148648 cell,
 * so this compiler expands that store address from the load and keeps the
 * pointer in $a2 for the whole first half; spelling the same store through an
 * inline D_80148648 instead makes gcc spend a `move` to preserve the address
 * (measured 86/91). The record bytes are flat symbols at their own addresses,
 * which is how the original encodes each of them (one 32-bit displacement
 * each); the +0x30 halfword is added before the +0xDB byte, which is what puts
 * the unsigned halfword in $a1 and the sign-extended byte in $v1 (measured
 * 81/91 and 86/91 for the reversed order). Residual detail: this build re-masks
 * the record index before the +0x04 store where the original stores first, and
 * reads the func_801D8690 first argument through the retained pointer instead
 * of the reloaded one; spelling every second-half access through the reloaded
 * cell instead costs the +0x04 store address (measured 84/91).
 */
void func_801E99AC(void) {
  u8* panel;
  u8  index;
  u32 offset;
  s16 value;
  u8  flags;

  panel = D_80148648;
  index = panel[0xA] - 3;
  offset = (u32)index * 0x118;

  *(s16*)&panel[4] =
      *(s16*)&D_801EB65E[offset] + *(s8*)&D_801EB712[offset] +
      (D_801EB638[offset] == 0 || D_801EB638[offset] == 3 ? D_801EAEA4
                                                         : D_801EAEA6);

  value = *(u16*)&D_801EB660[(u32)index * 0x118] +
          (s8)D_801EB713[(u32)index * 0x118] + 12;
  *(s16*)&D_80148648[6] = value;
  func_801D8690(*(s16*)&panel[4], value, D_80148648[0xA]);

  if (D_801462EF != 0) {
    flags = *D_801EB4D8;
    if ((flags & 0xC0) == 0) {
      if (flags == D_80148648[0xA]) {
        return;
      }
    } else if ((flags & 0x40) != 0) {
      return;
    }
  }
  commitQueuedPanelTask();
}
