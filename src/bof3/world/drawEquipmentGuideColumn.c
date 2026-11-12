#include "bof3/world/area03004_internal.h"

/* @source 0x801D88E0
 * @behavior draws the AREA030 equipment column of the status screen at the
 * requested origin: draws the two stacked status boxes at the origin through
 * drawStatusBoxPair, writes the five-character EQUIP header of the shared
 * LUREEQUIPGUIDE rodata run at (x + 0x33, y + 7) through func_8014F800, then
 * draws the two equipment rows selected by the shared index bytes D_80145028
 * (upper row) and D_80145026 (lower row). Each row takes that 20-byte record of
 * the shared equipment table 0x801C9E7C as a twelve-character name drawn at
 * (x + 0x1C, y + 0x1A) or (x + 0x1C, y + 0x2A) beside a func_800F7094 icon at
 * (x + 0x10, y + 0x1C) or (x + 0x10, y + 0x2C) that carries the record index,
 * the length 3 and a selection flag. Bit 7 of the work-record byte +0x07 draws
 * both rows plainly; while that bit is clear, bit 0 of the same byte selects the
 * active row, whose icon takes the selection flag 1 and whose name is drawn in
 * colour 7 with a second copy two pixels above it in the shade the work byte
 * itself holds (upper row) or its bit 1 (lower row). The column closes with the
 * five-character GUIDE header at (x + 0x33, y + 0x47) and, while the
 * work-record halfword +0x2C is not 0xFFFF, the guide icon of the main-RAM
 * offset table at the 0x80010000 base submitted at (0x19, 0x9A) with colour 0
 * and flag 0xFF.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The two index bytes are read differently: the upper one (D_80145028) reaches
 * both the icon calls and the name-pointer scales through an arm-scoped
 * pointer, which materialises its address once per arm into a callee-saved
 * register (the original materialises it into $s2, $s3 and $s0), while the
 * lower one (D_80145026) is read directly and folds into each of its loads.
 * Measured clean-C spellings of the same body: direct reads in all three arms
 * 228/292 instructions (1168 bytes, first difference the frame size and the
 * saved-register set), a function-scope pointer for both index bytes 166/292
 * (1132 bytes), a function-scope pointer for the upper byte alone 238/292
 * (1152 bytes), and arm-scoped pointers for both index bytes 268/292 (1160
 * bytes). The arm order is the original layout: bit 7 of the work byte draws
 * both rows plainly, the bit-0-clear arm follows it, and the bit-0-set arm
 * closes the chain.
 */
void drawEquipmentGuideColumn(s16 x, s16 y) {
  u8 flags;
  u16 guide;

  drawStatusBoxPair(x, y);
  func_8014F800((s16)(x + 0x33), (s16)(y + 7), 0, 5, (u32)D_801E21D4);
  flags = D_1F800044[7];
  if (flags & 0x80) {
    u8* idx = &D_80145028;
    func_800F7094((s16)(x + 0x10), (s16)(y + 0x1C), (*idx), 3, 0);
    func_8014F800((s16)(x + 0x1C), (s16)(y + 0x1A), 0, 0xC,
                  (u32)&D_801C9E7C[(*idx) * 20]);
    func_800F7094((s16)(x + 0x10), (s16)(y + 0x2C), D_80145026, 3, 0);
    func_8014F800((s16)(x + 0x1C), (s16)(y + 0x2A), 0, 0xC,
                  (u32)&D_801C9E7C[D_80145026 * 20]);
  } else if ((flags & 1) == 0) {
    u8* idx = &D_80145028;
    func_800F7094((s16)(x + 0x10), (s16)(y + 0x1C), (*idx), 3, 1);
    func_8014F800((s16)(x + 0x1C), (s16)(y + 0x1A), 7, 0xC,
                  (u32)&D_801C9E7C[(*idx) * 20]);
    func_800F7094((s16)(x + 0x10), (s16)(y + 0x1A), (*idx), 3, 0);
    func_8014F800((s16)(x + 0x1C), (s16)(y + 0x18), D_1F800044[7], 0xC,
                  (u32)&D_801C9E7C[(*idx) * 20]);
    func_800F7094((s16)(x + 0x10), (s16)(y + 0x2C), D_80145026, 3, 0);
    func_8014F800((s16)(x + 0x1C), (s16)(y + 0x2A), 0, 0xC,
                  (u32)&D_801C9E7C[D_80145026 * 20]);
  } else {
    u8* idx = &D_80145028;
    func_800F7094((s16)(x + 0x10), (s16)(y + 0x1C), (*idx), 3, 0);
    func_8014F800((s16)(x + 0x1C), (s16)(y + 0x1A), 0, 0xC,
                  (u32)&D_801C9E7C[(*idx) * 20]);
    func_800F7094((s16)(x + 0x10), (s16)(y + 0x2C), D_80145026, 3, 1);
    func_8014F800((s16)(x + 0x1C), (s16)(y + 0x2A), 7, 0xC,
                  (u32)&D_801C9E7C[D_80145026 * 20]);
    func_800F7094((s16)(x + 0x10), (s16)(y + 0x2A), D_80145026, 3, 0);
    func_8014F800((s16)(x + 0x1C), (s16)(y + 0x28), D_1F800044[7] & 2, 0xC,
                  (u32)&D_801C9E7C[D_80145026 * 20]);
  }
  func_8014F800((s16)(x + 0x33), (s16)(y + 0x47), 0, 5, (u32)D_801E21D9);
  guide = *(u16*)(D_1F800044 + 0x2C);
  if (guide != 0xFFFF) {
    func_8014F800(0x19, 0x9A, 0, 0xFF,
                  0x80010000u + PSX_PTR(u16, 0x80010000u)[guide]);
  }
}
