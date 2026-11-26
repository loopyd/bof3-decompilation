#include "bof3/battle/battle03_internal.h"

/* @source 0x801E9538
 * @behavior Takes byte +0x0A of the state object published at 0x80148648 as
 * the index of the 12-byte message slot table bound at 0x801EB4F4 (the message
 * view of the battleDispatchSlots entries: message pointer at +0x00, panel byte
 * at +0x06), counts the message tokens of that slot pointer through
 * countMessageTokens, forwards the state object halfwords +0x04 (minus 0x10)
 * and +0x06 to renderBattlePanelMessage, then draws through func_8014F800 the state
 * halfword +0x04 lowered by four per token and raised by 0x26, the state
 * halfword +0x06 raised by 3, the slot panel byte, style 0x12 and the same slot
 * message pointer.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderSlotMessagePanel(void) {
  u8 count;

  count = countMessageTokens(D_801EB4F4[((u8*)panelTaskRoot)[0xA]].unk_00);

  renderBattlePanelMessage(*(s16*)&((u8*)panelTaskRoot)[4] - 0x10,
                              *(s16*)&((u8*)panelTaskRoot)[6]);
  func_8014F800((s16)(*(u16*)&((u8*)panelTaskRoot)[4] - count * 4 + 0x26),
                (s16)(*(u16*)&((u8*)panelTaskRoot)[6] + 3),
                D_801EB4F4[((u8*)panelTaskRoot)[0xA]].unk_06, 0x12,
                (u32)D_801EB4F4[((u8*)panelTaskRoot)[0xA]].unk_00);
}
