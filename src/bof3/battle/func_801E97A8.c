#include "bof3/battle/battle03_internal.h"

/* @source 0x801E97A8
 * @status partial
 * @match 86.67
 * @residual live asm diff 78/90 instructions, 328 versus 360 bytes. First mismatch bnez v1,0x801e980c: the original keeps the maspsx division-trap expansion (break 7 / break 6) that the canonical maspsx pass omits, so this object needs an object-local -Wa,--expand-div profile. Second residual: the panel byte +0x03 reload is scheduled one instruction later than the original (sh 0x1C store before the cell reload here, after it there). Next rung: object-local expand-div profile, then load-order retry.
 */
/* @behavior Rescales the active enemy work record selected by panel byte +0x0A
 * into panel byte +0x0B (55/divisor ratio), forces that byte to one while the
 * record's halfword +0x94 is nonzero, mirrors halfwords +0x94/+0xA0 into panel
 * halves +0x14/+0x1C, clears panel byte +0x0D and advances panel byte +0x03.
 */
void func_801E97A8(void) {
  D_80148648[0xB] = (s32)D_801EB630[D_80148648[0xA] - 3].unk_94 * 55 /
                    (s32)D_801EB630[D_80148648[0xA] - 3].unk_a0;

  if ((D_801EB630[D_80148648[0xA] - 3].unk_94 != 0) &&
      (D_80148648[0xB] == 0)) {
    D_80148648[0xB] = 1;
  }

  FIELD_REF(u16, D_80148648, 0x14) = D_801EB630[D_80148648[0xA] - 3].unk_94;
  FIELD_REF(u16, D_80148648, 0x1C) = D_801EB630[D_80148648[0xA] - 3].unk_a0;
  D_80148648[0xD] = 0;
  D_80148648[3] += 1;
}
