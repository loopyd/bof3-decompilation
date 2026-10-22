#include "bof3/battle/battle15_internal.h"

/* @source 0x8009B03C */
/* @behavior Initialises the battle panel selection state at 0x80148570: writes
 * the byte at 0x801485B8 and the 0x24-byte-lower byte at 0x80148594 through a
 * 0x80148570 base, sets the D_801485B9/D_801485BA/D_801485BB bytes and the
 * D_801485BC/D_801485BE halfwords, the D_801485C2/D_801485C3/D_801485C5 bytes,
 * stores the 0x800B6F50 pointer in D_801485D8 and copies work byte +0x05 of
 * the record at 0x801EBF08 into D_801485C4 and 0x80148656, then clears the
 * head byte D_80148570 and the D_8014859C..D_801485A1 byte run, saves the
 * D_80148574/D_80148576 cursor pair into 0x80148598/0x8014859A and sets that
 * cursor to 0x140. Its only caller is 0x80099364.
 * @status partial
 * @match 73.97
 * @residual 54/73 instructions, 292 bytes matching the original's 292; first
 * mismatch at +0x0000. The original materialises one 0x801485B8 base register
 * across the whole initialisation (`sb a0,0(v1)` and `sb a0,-36(v1)`); this
 * candidate's base does not survive allocation, so both of those stores
 * rematerialise as absolute `lui $at; sb`, and the non-volatile stores are
 * emitted in a different order. Measured shapes: the previous
 * `(u8 *)&D_801485B8` base with `base - 0x24` matched 54 of 74 at 296 bytes;
 * the 0x80148570 base with 0x48/0x24 offsets used here matches 54 of 73 at 292
 * bytes; the same base as a plain `u8 *` with explicit volatile casts matches
 * 54 of 73 at 292 bytes. Nine earlier shapes with different store orders,
 * constant/value locals and pointer locals all measured 54 of 74 at 296
 * bytes, so the residual is allocator/scheduling of a 26-store sequence, not
 * source shape.
 */
void func_8009B03C(void) {
  u8 *work;
  volatile u8 *base;

  base = BATTLE_UNK_80148570_BASE;
  base[0x48] = 1;
  work = D_801EBF08;
  D_801485BA = 3;
  D_801485BB = 2;
  D_801485B9 = 8;
  D_801485C2 = 0;
  D_801485C3 = 0xFF;
  D_801485D8 = (u32)D_800B6F50;
  D_801485BC = -0xAA;
  D_801485C5 = 1;
  D_801485BE = 0x3F;
  D_801485C4 = work[5];
  base[0x24] = 1;
  D_80148597 = 1;
  {
    u16 old_x;
    u16 old_y;

    D_80148595 = 8;
    D_80148596 = 4;
    old_x = D_80148574;
    old_y = D_80148576;
    D_801485A0 = 0;
    D_8014859E = 0;
    D_8014859F = 0;
    D_8014859C = 0;
    D_801485A1 = 0;
    D_8014859D = 0;
    D_80148570.first = 0;
    D_80148574 = 0x140;
    D_80148598 = old_x;
    D_8014859A = old_y;
    D_80148656 = work[5];
  }
}
