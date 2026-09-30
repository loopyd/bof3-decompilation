#include "bof3/world/area02613_internal.h"

/* @source 0x801F2D5C
 * @behavior prepares a 0x20-byte projection scratch area through the shared
 * projection setup, then runs the four-step ring dispatch: for each of the four
 * ring steps it hands the work-cursor record centre block at +0xC, the record's
 * +0x1C scale word, the step angle and the two signed ring offset tables to the
 * local ring emitter func_801F2E04.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F2D5C(u32* arg0, u32 arg1) {
  u8 scratch[0x20];
  u8 i;

  func_801AFE18(scratch);

  i = 0;
  do {
    func_801F2E04((VECTOR*)arg0, (s32)arg1, (i << 10) & 0xFC00,
                  D_801F33FC[i], D_801F340C[i]);
    i++;
  } while (i < 4);
}
