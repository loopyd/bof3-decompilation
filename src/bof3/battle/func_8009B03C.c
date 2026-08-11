#include "bof3/battle/battle15_internal.h"

/* @source 0x8009B03C */
/* @behavior Initializes two related battle selection/control records. */
void func_8009B03C(void) {
  u8 *work;
  u8 *base;

  base = (u8 *)&D_801485B8;
  *(volatile u8 *)base = 1;
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
  *(volatile u8 *)(base - 0x24) = 1;
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
