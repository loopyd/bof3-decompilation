#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 panel-step handler, index 1 of the work-byte-3 dispatch
 * table 0x801E20C4, walking the second step-record chain at
 * panelStepRecordTableWorld00Area03004_801E20CC. While work byte 9 of the
 * record published at the scratchpad cursor 0x1F800044 is nonzero it decrements
 * that byte and, unless bit 7 of the stepped-from record's first byte is set,
 * runs the shared func_8014D978 helper. Once the countdown expires it looks the
 * step record up through work byte 0xA; when the record carries the 0xFF end
 * marker it re-seeds work byte 0xA from the record count byte and seeds the
 * display record (work byte 0xB takes 0x37 and work words 0x0C/0x10 take 0x6A).
 * It then requests the record's low 7 bits through func_8014D8D4 and, for the
 * record whose low 7 bits are 9, claims a front-end entry record through the
 * shared allocator func_8019601c and - unless the claimed index is 0xFF - seeds
 * that 0x74-byte row with flags 1, handler selector 0x1A at +5, bytes 1..3
 * 0x10/2/2 and the halfwords 0xF2/0x9E at +0x2E/+0x30; it finally arms work
 * byte 9 from the record count byte and advances work byte 0xA. Every call then
 * integrates the stepped-from record: work words 0x0C/0x10 gain the record
 * signed deltas and work byte 6 loses the second delta, and the sign of the
 * record fourth byte selects the func_801D6B28 marker mode. Once every sixteenth
 * frame (low nibble of the shared frame counter 0x80143E6C clear) with a
 * positive fourth byte it advances work byte 0xB and clamps it to 0x3C, formats
 * that byte through the "%2d" rodata string 0x801D0C04 into the shared
 * 0x80145AD4 buffer, and submits the buffer at (0xAC, 0xB0) through the shared
 * text drawer func_8014FF0C with mode 2 while frame-counter bit 2 is set and
 * mode 0 otherwise.
 * @source 0x801D6F34
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The published cursor cell 0x1F800044 is volatile, so every access group
 * re-reads it and keeps its own pointer local, as the record-family sibling
 * func_801D6CB0 does: the entry read, the end-marker re-seed (one read for the
 * display byte and one shared by the two word stores), the two post-call step
 * reads, the integration read, the marker read, the frame-gated counter read
 * with its own clamp re-read, and the formatting read. The entry-record seeding
 * statement order, the (u8) allocator result and the typed row stores come from
 * the exact sibling advancePanelStepResourceAndDrawBouncingMarkerWorld00Area03004_801D6128,
 * which seeds the same 0x80143FC8 row for its own step; this span's own trigger
 * (record request id 9), 0xFF index skip, halfwords 0xF2/0x9E and display tail
 * come from the original bytes.
 */
void advancePanelStepClaimEntryAndDrawCountValueWorld00Area03004_801D6F34(
    void) {
  u8* work;
  u8* cursor;
  u8* refresh;
  u8* seed;
  u8* count;
  u8* buf;
  s32 step;
  u8 slot;

  work = D_1F800044;
  if (work[9] == 0) {
    step = work[0xA];
    if ((s8)panelStepRecordTableWorld00Area03004_801E20CC[step].unk_00 == -1) {
      work[0xA] = panelStepRecordTableWorld00Area03004_801E20CC[step].count_01;
      D_1F800044[0xB] = 0x37;
      seed = D_1F800044;
      *(s32*)(seed + 0x0C) = 0x6A;
      *(s32*)(seed + 0x10) = 0x6A;
    }
    func_8014D8D4(
        panelStepRecordTableWorld00Area03004_801E20CC[D_1F800044[0xA]].unk_00 & 0x7F);
    if ((panelStepRecordTableWorld00Area03004_801E20CC[D_1F800044[0xA]].unk_00 & 0x7F) == 9) {
      slot = (u8)func_8019601c();
      if (slot != 0xFF) {
        D_80143FC8[slot].flags_00 = 1u;
        D_80143FC8[slot].byte_05 = 0x1A;
        D_80143FC8[slot].unk_01 = 0x10;
        D_80143FC8[slot].unk_02 = 2u;
        D_80143FC8[slot].unk_03 = 2u;
        D_80143FC8[slot].halfword_2E = 0xF2u;
        D_80143FC8[slot].halfword_30 = 0x9Eu;
      }
    }
    refresh = D_1F800044;
    refresh[9] =
        panelStepRecordTableWorld00Area03004_801E20CC[refresh[0xA]].count_01;
    refresh = D_1F800044;
    refresh[0xA] = refresh[0xA] + 1;
  } else {
    work[9] = work[9] - 1;
    if (((s8)panelStepRecordTableWorld00Area03004_801E20CC[D_1F800044[0xA] - 1]
             .unk_00 &
         0x80) == 0) {
      func_8014D978();
    }
  }

  cursor = D_1F800044;
  *(s32*)(cursor + 0x0C) =
      *(s32*)(cursor + 0x0C) +
      (s8)panelStepRecordTableWorld00Area03004_801E20CC[cursor[0xA] - 1].unk_02;
  *(s32*)(cursor + 0x10) =
      *(s32*)(cursor + 0x10) +
      (s8)panelStepRecordTableWorld00Area03004_801E20CC[cursor[0xA] - 1].unk_03;
  cursor[6] =
      cursor[6] -
      (u8)panelStepRecordTableWorld00Area03004_801E20CC[cursor[0xA] - 1].unk_02;
  if ((s8)panelStepRecordTableWorld00Area03004_801E20CC[D_1F800044[0xA] - 1].unk_03 <
      0) {
    func_801D6B28(0);
  } else {
    func_801D6B28(1);
  }
  if ((D_80143E6C & 0xF) == 0) {
    count = D_1F800044;
    if ((s8)panelStepRecordTableWorld00Area03004_801E20CC[count[0xA] - 1].unk_03 >
        0) {
      count[0xB] = count[0xB] + 1;
      count = D_1F800044;
      if (count[0xB] >= 0x3D) {
        count[0xB] = 0x3C;
      }
    }
  }
  buf = D_80145AD4;
  sprintf((char*)buf, (const char*)D_801D0C04, D_1F800044[0xB]);
  if (D_80143E6C & 4) {
    func_8014FF0C(0xAC, 0xB0, 2, buf);
  } else {
    func_8014FF0C(0xAC, 0xB0, 0, buf);
  }
}
