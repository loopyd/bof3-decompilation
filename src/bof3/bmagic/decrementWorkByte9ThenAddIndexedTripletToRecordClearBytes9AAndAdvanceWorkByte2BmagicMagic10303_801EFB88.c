#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */
extern u32 D_801F07AC[]; /* @source 0x801F07AC @kind data */
extern u32 D_801F07B0[]; /* @source 0x801F07B0 @kind data */
extern u32 D_801F07B4[]; /* @source 0x801F07B4 @kind data */

/* @source 0x801EFB88
 * @behavior Decrements work byte 9 and, when its truncated value reaches zero,
 *           adds the triplet indexed by work byte 0xB at stride 12 to words
 *           0x34, 0x38 and 0x3C of the record published at 0x8014638C, storing
 *           the sums into the work object. Clears work bytes 9 and 0xA and
 *           increments work byte 2. Re-reads the work pointer before the
 *           guarded stores, the byte 0xA clear and the final increment. The
 *           payload names this leaf through a table word at 0x801F07C4, not
 *           through a jal; it takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenAddIndexedTripletToRecordClearBytes9AAndAdvanceWorkByte2BmagicMagic10303_801EFB88(void) {
  u8 *work;
  u8 *record;
  u8 *next;
  u32 value;

  if (--g_battle_work[9] == 0) {
    work = g_battle_work;
    record = D_8014638C;
    *(u32 *)(work + 0x34) =
        *(u32 *)(record + 0x34) + D_801F07AC[work[0xB] * 3];
    *(u32 *)(work + 0x38) =
        *(u32 *)(record + 0x38) + D_801F07B0[work[0xB] * 3];
    value = *(u32 *)(record + 0x3C) + D_801F07B4[work[0xB] * 3];
    work[9] = 0;
    next = g_battle_work;
    *(u32 *)(work + 0x3C) = value;
    next[0xA] = 0;
    g_battle_work[2]++;
  }
}
