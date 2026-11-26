#include "bof3/battle/battle03_internal.h"

/* @source 0x801E960C
 * @behavior Dispatches the panel handler selected by state byte +0x03 through the
 * four-entry handler table copied off 0x801D0FD0, then forwards the state field
 * pointers +0xB/+0x1C/+0x14/+0xD/+0x18/+0x8 and the selected enemy work record's
 * +0x94/+0xA0 halfword pointers to callPanelForwardingTable. When checkBattlerInactiveFlag4000(state byte
 * +0x0A) reports a hit while state byte +0x0D is clear it sets state byte +0x03
 * to 3, and it finally raises bit 0x2000 of that enemy work record's flags
 * halfword +0x82 while its halfword +0x94 is below a quarter of its halfword
 * +0xA0, clearing the bit otherwise.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPanelFields(void) {
  Battle03FourDispatchTable local_20;
  u8*                       record;
  u32                       index;

  local_20 = D_801D0FD0;
  local_20.handlers[((u8*)panelTaskRoot)[3]]();

  index = ((u8*)panelTaskRoot)[0xA];
  record = D_801EB35C + index * 0x118;
  callPanelForwardingTable(((u8*)panelTaskRoot) + 0xB, (s16*)(record + 0x2C),
                (s16*)(((u8*)panelTaskRoot) + 0x1C), (s16*)(((u8*)panelTaskRoot) + 0x14),
                (s16*)(record + 0x20), ((u8*)panelTaskRoot) + 0xD,
                (s16*)(((u8*)panelTaskRoot) + 0x18), ((u8*)panelTaskRoot) + 0x8);

  if ((checkBattlerInactiveFlag4000(((u8*)panelTaskRoot)[0xA]) != 0) && (((u8*)panelTaskRoot)[0xD] == 0)) {
    ((u8*)panelTaskRoot)[3] = 3;
  }

  if (D_801EB630[((u8*)panelTaskRoot)[0xA] - 3].unk_94 <
      (D_801EB630[((u8*)panelTaskRoot)[0xA] - 3].unk_a0 >> 2)) {
    D_801EB630[((u8*)panelTaskRoot)[0xA] - 3].unk_82 |= 0x2000u;
  } else {
    D_801EB630[((u8*)panelTaskRoot)[0xA] - 3].unk_82 &= 0xdfffu;
  }
}
