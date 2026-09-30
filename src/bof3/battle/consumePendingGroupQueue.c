#include "bof3/battle/battle03_internal.h"

/* @source 0x801DD29C
 * @behavior Drains the pending group queue from its cursor: while the signed
 * cursor byte at `groupQueueCursor` is below three, takes the queued group id
 * at `groupQueueTable[cursor]`, stops at the empty-slot marker 0xFF, otherwise
 * publishes the id into the selection byte 0x15 below the cursor and arms that
 * group's local work record (`D_80145E90[id]` bytes +0x118 = 3, +0x119 = 1)
 * before advancing the cursor.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DD29C(void) {
  s8* cursor;
  u8 group;
  u8 empty_marker;
  u8 record_flag;
  u8 record_state;

  if (*(s8*)&groupQueueCursor >= 3) {
    return;
  }
  cursor = (s8*)&groupQueueCursor;
  empty_marker = 0xFF;
  record_flag = 1;
  record_state = 3;
loop:
  group = groupQueueTable[*cursor];
  if (group == empty_marker) {
    return;
  }
  /* The selection byte 0x801462EE sits 0x15 bytes below the cursor; func_801DF3F8
   * reads it as the group index this queue selected. */
  *(u8*)(cursor - 0x15) = group;
  D_80145E90[group].unk_119 = record_flag;
  D_80145E90[*(u8*)(cursor - 0x15)].unk_118 = record_state;
  *(u8*)cursor = *(u8*)cursor + 1;
  if (*cursor < 3) {
    goto loop;
  }
}
