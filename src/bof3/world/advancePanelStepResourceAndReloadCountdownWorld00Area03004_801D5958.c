#include "bof3/world/area03004_internal.h"

/* Two-byte AREA030 panel-step records: `resource` is requested through the
 * shared func_8014D6B8 helper and `countdown` is reloaded into work byte 9.
 * The last record's resource byte is the 0xFF end marker and its countdown byte
 * restarts the sequence at that index. */
typedef struct Area030PanelStepRecord {
  u8 resource;
  u8 countdown;
} Area030PanelStepRecord;

extern Area030PanelStepRecord panelStepResourceTableWorld00Area03004_801E1FE8[]; /* @source 0x801E1FE8 @kind table */

/* @source 0x801D5958
 * @behavior AREA030 panel-step resource handler (handler index 1 of the
 * work-byte-3 table at 0x801E1FD4): while byte 9 of the work record published
 * at the scratchpad cursor 0x1F800044 is nonzero it decrements that byte and
 * runs the shared func_8014D978 helper; otherwise it reads byte 0xA of the same
 * record as an index into the two-byte record table at 0x801E1FE8, restarts the
 * index from that entry's countdown byte when the entry's resource byte is the
 * 0xFF end marker, requests the entry resource through func_8014D6B8, reloads
 * work byte 9 from the entry countdown, increments work byte 0xA and jumps to
 * the shared func_8014D4E0 helper.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advancePanelStepResourceAndReloadCountdownWorld00Area03004_801D5958(void) {
  u8 *work;
  u8 *reload;
  u8 *cursor;
  u8 step;

  work = D_1F800044;
  if (work[9] == 0) {
    step = work[0xA];
    if (panelStepResourceTableWorld00Area03004_801E1FE8[step].resource == 0xFF) {
      work[0xA] = panelStepResourceTableWorld00Area03004_801E1FE8[step].countdown;
    }
    /* Separate quantity for this reload point: the original keeps the call
     * group's cursor in its own register and coalesces the shifted index into
     * it, while the two later store groups share one cursor register. */
    cursor = D_1F800044;
    func_8014D6B8(panelStepResourceTableWorld00Area03004_801E1FE8[cursor[0xA]].resource);
    reload = D_1F800044;
    reload[9] = panelStepResourceTableWorld00Area03004_801E1FE8[reload[0xA]].countdown;
    reload = D_1F800044;
    reload[0xA] = reload[0xA] + 1;
  } else {
    work[9] = work[9] - 1;
    func_8014D978();
  }
  func_8014D4E0();
}
