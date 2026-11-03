#include "bof3/world/area03004_internal.h"

/* @behavior marks and dispatches the AREA030 entry selected by scratch byte 6: when
 * the fourth byte of that 0x801E1F20 row is set it takes the front-end pick returned
 * by func_8019601C and, unless that pick is -1, arms the shared entry record at
 * 0x80143FC8 with flags 1, byte 0x01 8, byte 0x05 15 and byte 0x06 equal to the
 * scratch selector, clearing bytes 0x0B and 0x4A; the work index then becomes 6.
 * While the row byte is clear it copies the row's first byte into work byte 0x4B and
 * advances the work index. The fixed inset panel rectangle is requested last.
 * @source 0x801D36F8
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D36F8(void) {
  u8* cursor;
  u8* next;
  s8 pick;
  u8 selector;

  cursor = (u8*)D_1F800044;
  if (D_801E1F20[cursor[6u]][3u] != 0u) {
    pick = (s8)func_8019601c();
    if (pick != -1) {
      D_80143FC8[pick].flags_00 = 1u;
      D_80143FC8[pick].byte_05 = 15u;
      selector = D_1F800044[6u];
      D_80143FC8[pick].byte_4A = 0u;
      D_80143FC8[pick].byte_0B = 0u;
      D_80143FC8[pick].unk_01 = 8u;
      D_80143FC8[pick].byte_06 = selector;
    }
    next = (u8*)D_1F800044;
    next[1u] = 6u;
  } else {
    cursor[0x4Bu] = D_801E1F20[cursor[6u]][0u];
    next = (u8*)D_1F800044;
    next[1u] = (u8)(next[1u] + 1u);
  }
  func_801D9534(20, 18, 280, 19, 0);
}
