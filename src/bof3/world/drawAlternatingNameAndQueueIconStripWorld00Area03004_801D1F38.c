#include "bof3/world/area03004_internal.h"

/* @source 0x801D1F38
 * @behavior AREA030 equipment-name step, entry 3 of the work-record handler table
 * 0x801E1D10 selected by work-record byte 1: bit 6 of the shared frame counter
 * D_80143E6C picks which of the shared equipment index bytes
 * 0x80145028/0x80145026 is drawn, that 20-byte record of the shared
 * main-executable equipment table 0x801C9E7C is submitted as a
 * twelve-character name run at (0x8A, 0xC8) through drawEncodedTextAt, and the
 * signed byte difference between the message limit byte 0x80143FCF and the
 * active record byte 0x0F is clamped to 0..4 and, while it is nonzero, queues
 * the two-icon strip at (0x80143FF6 + 0x0A, 0x80143FF8 - 0x18) through
 * queueIconStrip with that value as the second icon variant.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawAlternatingNameAndQueueIconStripWorld00Area03004_801D1F38(void) {
  s8 level;
  s32 selected;
  s8 clamped;
  u8* record;

  if (D_80143E6C & 0x40) {
    drawEncodedTextAt(0x8A, 0xC8, 0, 0xC, (u32)&D_801C9E7C[D_80145028 * 20]);
  } else {
    drawEncodedTextAt(0x8A, 0xC8, 0, 0xC, (u32)&D_801C9E7C[D_80145026 * 20]);
  }
  record = D_801E3210;
  selected = *(u8*)&D_80143FCF;
  selected -= record[0x0F];
  level = selected;
  selected = (u32)selected << 24;
  if ((selected >> 24) >= 0) {
    if ((selected >> 24) >= 5) {
      clamped = 4;
    } else {
      clamped = level;
    }
  } else {
    clamped = 0;
  }
  level = clamped;
  if ((s8)level == 0) {
    return;
  }
  queueIconStrip(D_80143FF6 + 0xA, D_80143FF8 - 0x18, 2, level);
}
