#include "bof3/battle/battle03_internal.h"

/* @source 0x801E97A8
 * @behavior Selects D_801EB630[panel byte +0x0A - 3] and stores 55 times its
 * unsigned halfword +0x94 divided by halfword +0xA0 in panel byte +0x0B. If
 * +0x94 is nonzero and the stored byte is zero, replaces the byte with one.
 * Copies those halfwords into panel +0x14 and +0x1C, clears panel byte +0x0D
 * and increments panel byte +0x03. The panel pointer is D_80148648.
 * @status partial
 * @match 86.67
 * @residual live asm diff 78/90 instructions, 328 versus 360 bytes. First mismatch bnez v1,0x801e980c: the original keeps the maspsx division-trap expansion (break 7 / break 6) that the canonical maspsx pass omits. A disposable flag-search probe with an -O2 -Wa,--expand-div candidate measured 88/90 (97.8%) and clears that whole block, so the object needs an object-local -Wa,--expand-div profile; no entry was recorded because the lane authorizes one only for an exact match, and the remaining 2 instructions still differ here: the panel byte +0x03 reload (lui/lw of D_80148648) is scheduled after the 0x1C halfword store instead of before it, which also costs the load-delay slot. Next rung: object-local expand-div profile plus a load-order retry for that final block.
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
