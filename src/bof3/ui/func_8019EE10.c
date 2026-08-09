#include "bof3/ui/game00_internal.h"

/**
 * @source 0x8019EE10
 * @behavior Initialize fields in the current game work record from shared state.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_8019EE10(void)
{
  u8* work;
  u8* next;

  ((u8*)g_game_work)[0x24] = 1;
  *(u16*)&((u8*)g_game_work)[0x2C] = PSX_REF(u16, 0x80145EBCu);
  ((u8*)g_game_work)[0x28] = 2;
  *(u32*)&((u8*)g_game_work)[0x4C] = PSX_REF(u32, 0x80145EDCu);
  ((u8*)g_game_work)[0x2B] = 1;
  ((u8*)g_game_work)[0x29] = PSX_REF(u8, 0x80145EB9u);
  work = (u8*)g_game_work;
  work[0x25] = 0x1D;
  next = (u8*)g_game_work;
  *(u32*)&work[0x70] = 0;
  next[0x26] = 0;
  ((u8*)g_game_work)[0x5D] = 0;
  ((u8*)g_game_work)[0x5E] = 0;
  ((u8*)g_game_work)[0x5F] = 0;
  ((u8*)g_game_work)[0x5C] = 0;
  ((u8*)g_game_work)[0x48] = 0;
}
