#include "bof3/scenario/sce10eff_internal.h"

extern int rand(void);

/* @source 0x801D1034
 * @behavior Reseeds the scratchpad work object: stores `rand() & 0xF` in its
 * byte 11, clears bytes 9 and 10, copies the word pair at offsets 0x34/0x38
 * from the object cached at 0x801D2744 into its own 0x34/0x38, stores the
 * distance between that pair (returned by func_8015477C) at offset 0x3E and
 * advances byte 3.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D1034(void) {
  u8* source;
  s32 coord_y;

  ((u8*)D_1F800044)[0x0b] = (u8)(rand() & 0x0f);
  ((u8*)D_1F800044)[0x09] = 0;
  ((u8*)D_1F800044)[0x0a] = 0;
  source = D_801D2744;
  D_1F800044->unk_34 = *(s32*)(source + 0x34);
  coord_y = *(s32*)(source + 0x38);
  D_1F800044->unk_38 = coord_y;
  D_1F800044->unk_3e = (u16)func_8015477C(D_1F800044->unk_34, coord_y);
  ((u8*)D_1F800044)[0x03]++;
}
