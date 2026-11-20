#include "bof3/world/area03004_internal.h"

/* Three-byte AREA030 panel-step records at 0x801E2024, the two-byte record
 * layout of the step table 0x801E1FE8 widened by a mode byte: `resource` supplies the active-work sequence selector passed to selectActiveWorkSequence, `countdown` is reloaded
 * into work byte 9 and doubles as the next step index when `resource` is zero
 * (the table's last record is all zero), and `mode` carries the step's marker
 * behaviour - 0x50 stops the work byte 6 offset from moving while the table's
 * other records hold 0, 1 or -1. */
typedef struct Area030PanelStepModeRecord {
  s8 resource;
  u8 countdown;
  s8 mode;
} Area030PanelStepModeRecord;

/* Unsigned view of the same records: the sequence selector passed to selectActiveWorkSequence is
 * read with an `lbu` while the zero test above reads the byte signed. */
typedef struct Area030PanelStepModeRecordBytes {
  u8 resource;
  u8 countdown;
  u8 mode;
} Area030PanelStepModeRecordBytes;

extern Area030PanelStepModeRecord panelStepModeTableWorld00Area03004_801E2024[]; /* @source 0x801E2024 @kind table */

/* @source 0x801D6128
 * @behavior AREA030 panel-step sequence handler, index 1 of the work-byte-3 dispatch
 * table 0x801E2014: while work byte 9 of the record published at the
 * scratchpad cursor 0x1F800044 is nonzero it decrements that byte and runs the
 * shared func_8014D978 helper; otherwise it reads work byte 0xA as an index
 * into the three-byte record table 0x801E2024, restarts the index from that
 * record's countdown byte and clears work byte 6 when the record's resource
 * byte is zero, selects the sequence identified by the record through
 * selectActiveWorkSequence, reloads work byte 9 from the record countdown, and
 * when the record mode byte is 0x50 arms the shared entry record 0x80143FC8
 * selected by the front-end pick of findFreeRecord with flags 1, byte 0x05
 * 0x1A, byte 0x01 0x10, bytes 0x02/0x03 2 and the halfwords 0xF0/0xA6 at
 * 0x2E/0x30, before advancing work byte 0xA. Every call then runs
 * queueUnprojectedActiveWork; unless the stepped-from record's mode byte is
 * 0x50 it adds that signed mode byte to work byte 6 once every fourth frame
 * and clamps the byte to 0..0x18. Finally it draws the 16x32 Gouraud quad at
 * (0xEC, 0x70), the 16x16 sprite at (0xEC, 0x80), texture-page draw mode 0/1,
 * the sprite at (0x14, 1, 0xE4, 0x68) and, for every nonzero mode byte, the
 * 8x8 marker sprite at (0xFC, 0x70 + work byte 6).
 * @status exact
 * @match 100.00
 * @residual none
 */
void advancePanelStepSequenceAndDrawBouncingMarkerWorld00Area03004_801D6128(void) {
  u8* work;
  u8* cursor;
  u8* reload;
  u8* check;
  u8* next;
  u8* stepped;
  u8* bounce;
  u8* marker;
  s8 mode;
  s8 level;
  s8 height;
  u8 slot;
  u8 step;

  work = D_1F800044;
  if (work[9] == 0) {
    step = work[0xA];
    if (panelStepModeTableWorld00Area03004_801E2024[step].resource == 0) {
      work[0xA] = panelStepModeTableWorld00Area03004_801E2024[step].countdown;
      D_1F800044[6] = 0;
    }
    cursor = D_1F800044;
    selectActiveWorkSequence(((Area030PanelStepModeRecordBytes*)
                        panelStepModeTableWorld00Area03004_801E2024)[cursor[0xA]]
                      .resource);
    reload = D_1F800044;
    reload[9] = panelStepModeTableWorld00Area03004_801E2024[reload[0xA]].countdown;
    check = D_1F800044;
    if (panelStepModeTableWorld00Area03004_801E2024[check[0xA]].mode == 0x50) {
      slot = (u8)findFreeRecord();
      WORLD00_AREA030_ARM_PANEL_ENTRY(slot);
      D_80143FC8[slot].unk_02 = 2u;
      D_80143FC8[slot].unk_03 = 2u;
      D_80143FC8[slot].halfword_2E = 0xF0u;
      D_80143FC8[slot].halfword_30 = 0xA6u;
    }
    next = D_1F800044;
    next[0xA] = next[0xA] + 1;
  } else {
    work[9] = work[9] - 1;
    func_8014D978();
  }

  queueUnprojectedActiveWork();
  stepped = D_1F800044;
  mode = panelStepModeTableWorld00Area03004_801E2024[stepped[0xA] - 1].mode;
  if (mode != 0x50 && (D_80143E6C & 3) == 0) {
    stepped[6] = stepped[6] + mode;
    bounce = D_1F800044;
    height = *(s8*)(bounce + 6);
    if (height >= 0) {
      if (height < 0x19) {
        level = height;
      } else {
        level = 0x18;
      }
    } else {
      level = 0;
    }
    bounce[6] = level;
  }

  drawBlueGradientQuad16x32World00Area03004(0xEC, 0x70, 1u);
  drawCounterIndexedSprite16x16World00Area03004(0xEC, 0x80, 1u);
  submitTpageDrawMode(0, 1);
  appendIndexedSpriteWorld00Area03004(0x14, 1, 0xE4, 0x68);
  marker = D_1F800044;
  if (panelStepModeTableWorld00Area03004_801E2024[marker[0xA] - 1].mode != 0) {
    drawFixedCellSprite8x8World00Area03004(0xFC, marker[6] + 0x70, 1u);
  }
}
