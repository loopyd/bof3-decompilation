#include "bof3/world/area03004_internal.h"

/* @behavior initializes the AREA030 panel step: requests resource 43 through the
 * shared func_8014DD3C helper, stores the halfwords 0x186 at work offset 0x2E and
 * 0x6E at 0x30, writes 0x80 to byte 0x24, 2 to byte 0x29, clears bytes 0x48, 0x5D,
 * 0x5E, 0x5F and 0x2A, then requests the entry-mode byte 6 through func_8014D6B8 and
 * dispatches the halfword of the 0x801E1DA0 table selected by that byte through
 * func_8015DF18 unless it is 0xFFFF, finally advancing the dispatch index.
 * @source 0x801D339C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D339C(void) {
  u8* first;
  u8* next;
  u8* third;
  u8* fourth;
  u8* mode;
  u16 cue;

  func_8014DD3C(43);

  first = (u8*)D_1F800044;
  *(volatile s16*)(first + 0x2Eu) = 390;
  first[0x24u] = 128u;
  next = (u8*)D_1F800044;
  *(volatile s16*)(first + 0x30u) = 110;
  next[0x48u] = 0u;
  third = (u8*)D_1F800044;
  third[0x29u] = 2u;
  ((u8*)D_1F800044)[0x5Du] = 0u;
  ((u8*)D_1F800044)[0x5Eu] = 0u;
  ((u8*)D_1F800044)[0x5Fu] = 0u;
  ((u8*)D_1F800044)[0x2Au] = 0u;

  mode = (u8*)D_1F800044;
  func_8014D6B8(mode[6u]);

  cue = D_801E1DA0[((u8*)D_1F800044)[6u]];
  if (cue != 0xFFFFu) {
    func_8015DF18(cue);
  }

  fourth = (u8*)D_1F800044;
  fourth[1u] = (u8)(fourth[1u] + 1u);
}
