#include "bof3/battle/battle15_internal.h"

/* @behavior Draws the three battle panel slots with their cursor sprites and
 * label records. When the selection byte at 0x801462E5 has both its high bit
 * and the loop slot set, that slot is redrawn with the dedicated cursor label
 * and its own two-byte label record; otherwise the shared label row is used.
 * After the loop the same byte selects the final cursor pass.
 * @source 0x800B0B0C
 * @status partial
 * @match 44.92
 * @residual non-exact live audit: 53/118 instructions; 472 original bytes
 *   versus 456 current. Argument widths, save set, loop rotation and the two
 *   merged call tails match. Residual class is allocation: the original keeps
 *   the u16 view of the base y in a stack slot and reloads it with `lhu`, keeps
 *   the slot offset (slot * 0x30) in its own register, and numbers the saved
 *   registers s0-s8 in a different order. Repair round measured the reviewed
 *   width lever: a u16 base_y parameter (private prototype changed in the same
 *   transaction, then reverted) with four call-site spellings, including a
 *   named slot offset and an offset-first cursor x. All four canonicalised to
 *   this same 53/118 and 456 bytes with the 72-versus-64 frame intact, so the
 *   parameter width is not the cause of the missing entry sh/lhu pair.
 */
void func_800B0B0C(s16 base_x, s16 base_y) {
  s16 cursor_y;
  u16 slot_x;
  u8  slot;
  u8  active;
  u8  selection;

  cursor_y = base_y + 3;
  slot = 0;
  do {
    slot_x = base_x + slot * 0x30;
    func_801AE3F0(slot_x, base_y, 0x2D, 0x14, 0, D_80144952);
    selection = D_801462E5;
    if (selection & 0x80) {
      active = selection & 0x7F;
      if (slot == active) {
        func_8014F800((s16)(slot_x + 4), cursor_y, 0, 0x10,
                      (u32)&D_800B6D30[slot * 3]);
        func_801AF390(slot_x, base_y, D_800B6D28, 1);
      } else {
        func_8014F800((s16)(slot_x + 4), cursor_y, 7, 0x10,
                      (u32)&D_800B6D30[slot * 3]);
        func_801AF390(slot_x, base_y, D_800B6D20, 1);
      }
    } else {
      func_8014F800((s16)(slot_x + 4), cursor_y, 0, 0x10,
                    (u32)&D_800B6D30[slot * 3]);
      func_801AF390(slot_x, base_y, D_800B6D20, 1);
    }
    slot++;
  } while (slot < 3);

  selection = D_801462E5;
  if (!(selection & 0x80)) {
    active = selection & 0x7F;
    func_801647C4((u16)(base_x + active * 0x30), (u16)(base_y + 4), 0);
  }
}
