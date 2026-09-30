#include "bof3/battle/battle03_internal.h"

/* @source 0x801E9538
 * @behavior Takes byte +0x0A of the state object published at 0x80148648 as
 * the index of the 12-byte message slot table bound at 0x801EB4F4 (the message
 * view of the battleDispatchSlots entries: message pointer at +0x00, panel byte
 * at +0x06), counts the message tokens of that slot pointer through
 * countMessageTokens, forwards the state object halfwords +0x04 (minus 0x10)
 * and +0x06 to func_801D8270, then draws through func_8014F800 the state
 * halfword +0x04 lowered by four per token and raised by 0x26, the state
 * halfword +0x06 raised by 3, the slot panel byte, style 0x12 and the same slot
 * message pointer.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E9538(void) {
  u8 count;

  count = countMessageTokens(D_801EB4F4[D_80148648[0xA]].unk_00);

  func_801D8270(*(s16*)&D_80148648[4] - 0x10, *(s16*)&D_80148648[6]);
  func_8014F800((s16)(*(u16*)&D_80148648[4] - count * 4 + 0x26),
                (s16)(*(u16*)&D_80148648[6] + 3),
                D_801EB4F4[D_80148648[0xA]].unk_06, 0x12,
                (u32)D_801EB4F4[D_80148648[0xA]].unk_00);
}
