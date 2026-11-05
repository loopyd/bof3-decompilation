#include "bof3/battle/battle03_internal.h"

/* Class-keyed byte tables read by the scratch work initializer at 0x801E679C.
 * D_801EB4A4 holds one page byte per class and feeds scratch byte +0x27 through
 * the 0xB0 base. D_801EB4B0 holds a two-byte row per class: the half-row byte
 * +0x08 selects the column and the low bit of the same byte selects the +0x2A
 * flag. */
/* @source 0x801EB4A4 @kind table */
extern u8 D_801EB4A4[];
/* @source 0x801EB4B0 @kind table */
extern u8 D_801EB4B0[][2];
void func_8014D6B8(u8 request); /* @source 0x8014D6B8 */

/* @source 0x801E679C
 * @behavior Handler index 0 of the two-entry table selected by scratch work
 * byte +0x01: initializes the scratch work display bytes +0x24..+0x2C, taking
 * +0x27 from the class-keyed page table D_801EB4A4 plus 0xB0 and raising +0x2A
 * when the class byte +0x07 is 6 with an odd half-row byte +0x08, or when that
 * byte minus one is below 2, then advances work byte +0x01 and requests the
 * class row and half-row column byte of D_801EB4B0 before the shared idle
 * helper.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E679C(void)
{
  Battle03LocalWork* work;
  Battle03LocalWork* next_work;

  D_1F800044->unk_29 = 0;
  ((u8*)D_1F800044)[0x25] = 0x1d;
  ((u8*)D_1F800044)[0x26] = 0;
  ((u8*)D_1F800044)[0x24] = 0x84;
  ((u8*)D_1F800044)[0x27] = D_801EB4A4[((u8*)D_1F800044)[0x07]] + 0xb0;
  ((u8*)D_1F800044)[0x28] = 0;
  work = D_1F800044;
  work->unk_2b = 0;
  next_work = D_1F800044;
  FIELD_REF(u16, work, 0x2c) = 0;
  if (((u8*)next_work)[0x07] != 6) {
    if ((u32)(((u8*)next_work)[0x08] - 1) < 2) {
      next_work->unk_2a = 1;
    } else {
      next_work->unk_2a = 0;
    }
  } else {
    if (next_work->unk_08 & 1) {
      next_work->unk_2a = 1;
    } else {
      next_work->unk_2a = 0;
    }
  }
  D_1F800044->unk_01++;
  func_8014D6B8(D_801EB4B0[((u8*)D_1F800044)[0x07]]
                          [((u8*)D_1F800044)[0x08] >> 1]);
  func_8014D4E0();
}
