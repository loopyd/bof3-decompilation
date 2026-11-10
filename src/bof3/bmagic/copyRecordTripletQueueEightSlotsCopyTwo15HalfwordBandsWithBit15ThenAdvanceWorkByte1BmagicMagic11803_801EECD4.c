#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */
extern u16 D_8003AC00; /* @source 0x8003AC00 @kind data */
extern u16 D_8003AC20; /* @source 0x8003AC20 @kind data */
extern u8 D_80145988; /* @source 0x80145988 @kind data */
extern u32 func_801E590C(u32 kind, u32 mode); /* @source 0x801E590C */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EECD4
 * @behavior Copies words 0x34, 0x38 and 0x3C from the record pointer at
 *           0x8014638C into the scratchpad work object and clears work byte B.
 *           Claims eight action slots with func_801E590C(1, 0x2F), stamps
 *           each slot byte 9 with its one-based ordinal times four, publishes
 *           the work pointer at slot word 0x74, and increments work byte B.
 *           Copies fifteen halfwords in each of two bands from 0x80036C02
 *           and 0x80036C22 to 0x8003AC02 and 0x8003AC22 with bit 15 set,
 *           clears both destination leading halfwords, sets byte 0x80145988
 *           to one, invokes func_8015D71C(0x100), clears work byte 9 and
 *           advances work byte 1. Entry zero of the five-word handler run at
 *           0x801EEC04, indexed by work byte 1 in func_801EEC18. Unknown
 *           record fields, table roles and the helper role stay anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyRecordTripletQueueEightSlotsCopyTwo15HalfwordBandsWithBit15ThenAdvanceWorkByte1BmagicMagic11803_801EECD4(void) {
  u8 *record;
  u8 *work;
  u8 *slotWork;
  u8 *countWork;
  u32 i;
  u32 offset;
  u16 *dst;
  u16 *src;

  record = D_8014638C;
  work = g_battle_work;
  *(u32 *)&work[0x34] = *(u32 *)&record[0x34];
  *(u32 *)&work[0x38] = *(u32 *)&record[0x38];
  *(u32 *)&work[0x3C] = *(u32 *)&record[0x3C];
  work[0xB] = 0;
  i = 0;
  do {
    offset = (u8)func_801E590C(1, 0x2F) * 0x78u;
    slotWork = g_battle_work;
    D_801EC339[offset] = (i + 1) * 4;
    countWork = g_battle_work;
    *(u32 *)&D_801EC3A4[offset] = (u32)slotWork;
    countWork[0xB]++;
    i++;
  } while (i < 8);

  i = 1;
  dst = PSX_PTR(u16, 0x8003AC02u);
  src = PSX_PTR(u16, 0x80036C02u);
  do {
    dst[0] = src[0] | 0x8000;
    dst[0x10] = src[0x10] | 0x8000;
    src++;
    i++;
    dst++;
  } while (i < 0x10);
  D_8003AC00 = 0;
  D_8003AC20 = 0;
  D_80145988 = 1;
  func_8015D71C(0x100);
  g_battle_work[9] = 0;
  g_battle_work[1]++;
}
