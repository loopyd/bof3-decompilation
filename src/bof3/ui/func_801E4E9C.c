#include "bof3/ui/shop00_internal.h"

/* @source 0x801E4E9C
 * @behavior shop panel emitter reached from the panel state dispatcher
 *           func_801E440C, which forwards the global panel task root: emits the
 *           panel backing through func_801AE3F0 with the task's x/field-6
 *           halfwords offset by 3, the constants 0x72 and 0x58, the constant 0
 *           and the main-RAM CLUT-bank byte D_80144952. It then walks the eight
 *           0xA4-byte-stride field records at D_80144952 + 0x1B, and for every
 *           record whose byte at +2 has bit 0 set and whose byte at +0x16
 *           matches the main-RAM byte D_80148361 it emits the record through
 *           func_801E4FC8 at the task's x/field-6 coordinates plus a running
 *           (x, y) icon offset (record byte at +0 as the third argument).
 *           The x offset steps by 0x25 and wraps to 0 with the y offset
 *           stepping by 0x2A once it reaches 0x4B. Finally it draws the panel
 *           frame through func_801DD350 with the task's x/field-6 halfwords and
 *           the constants 0xD and 0xA.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E4E9C(PanelTask* task) {
  u8* bank;
  u8* entry;
  u8* rec;
  s16 x;
  s32 y;
  u8 i;

  bank = &D_80144952;
  entry = bank + 0x1B;
  x = 0;
  y = 0;
  func_801AE3F0(task->x + 3, task->field_06 + 3, 0x72, 0x58, 0, *bank);
  for (i = 0; i < 8; i++) {
    rec = entry + i * 0xA4;
    if ((rec[2] & 1) != 0 && rec[0x16] == D_80148361) {
      func_801E4FC8(task->x + x + 4, task->field_06 + y + 5, rec[0], 0);
      x = x + 0x25;
      if (x >= 0x4B) {
        x = 0;
        y = y + 0x2A;
      }
    }
  }
  func_801DD350(task->x, task->field_06, 0xD, 0xA);
}
