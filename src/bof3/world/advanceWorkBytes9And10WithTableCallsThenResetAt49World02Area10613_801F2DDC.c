#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */
extern u8 D_801F3E90[]; /* @source 0x801F3E90 @kind data */
extern u8 D_801F3EBC[]; /* @source 0x801F3EBC @kind data */
extern u8 D_801F3ED8[]; /* @source 0x801F3ED8 @kind data */

extern void func_801F2F38(s32 arg0, s32 arg1); /* @source 0x801F2F38 */
extern void func_801F31DC(s32 arg0, s32 arg1); /* @source 0x801F31DC */
extern void func_801F33D4(s32 arg0); /* @source 0x801F33D4 */
extern void func_8015B580(void* arg0, u8 bit_index); /* @source 0x8015B580 */
extern void func_8015507C(s32 arg0, s32 arg1); /* @source 0x8015507C */
extern s32 func_8015B5D4(u32 arg0, s32 arg1); /* @source 0x8015B5D4 */
extern void func_80196070(void); /* @source 0x80196070 */

/* @source 0x801F2DDC
 * @behavior Dispatch-table callback at 0x801F3EB8. When work byte +9 is
 *           nonzero, calls 0x801F2F38 with 1 and table 0x801F3E90 indexed by
 *           that byte and scaled by 16, then advances the byte. At 23 it
 *           calls 0x8015B580 on bank 0x80144F28 with 0x50, calls 0x8015507C
 *           with 0x13 and 0x25, and clears byte +9. When 0x8015B5D4 reports
 *           bank index 0x4F set, calls 0x801F31DC from tables 0x801F3EBC
 *           and 0x801F3ED8 for byte +10 below 25, then calls 0x801F33D4
 *           with byte +10. Advances byte +10 unconditionally and calls
 *           0x80196070 at 49 or above. The work pointer is reloaded from
 *           scratchpad cell 0x1F800044 at each original access group.
 *           Takes no arguments and returns nothing; callee roles remain
 *           address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkBytes9And10WithTableCallsThenResetAt49World02Area10613_801F2DDC(void) {
  u32 step;
  u32 index;

  step = D_1F800044[9];
  if (step != 0) {
    func_801F2F38(1, D_801F3E90[step] << 4);
    D_1F800044[9]++;
  }
  if (D_1F800044[9] == 23) {
    func_8015B580(&D_80144F28, 0x50);
    func_8015507C(0x13, 0x25);
    D_1F800044[9] = 0;
  }
  if (func_8015B5D4((u32)&D_80144F28, 0x4F) != 0) {
    index = D_1F800044[10];
    if (index < 25) {
      func_801F31DC(D_801F3EBC[index] << 2, D_801F3ED8[index >> 1]);
    }
    func_801F33D4(D_1F800044[10]);
  }
  D_1F800044[10]++;
  if (D_1F800044[10] >= 49) {
    func_80196070();
  }
}
