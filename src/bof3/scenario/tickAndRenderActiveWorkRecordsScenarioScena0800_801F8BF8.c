#include "bof3/bof3.h"

typedef void (*ScenarioScena0800WorkRecordHandler)(u8* record);

extern ScenarioScena0800WorkRecordHandler D_801FE8FC[];
extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

void func_8014E5A0(u8 arg0, u8 arg1);
void func_801AFE18(void* arg0);
void drawTranslucentGouraudDiscOfSixteenTrianglesAroundProjectedRecordPointScenarioScena0800_801F8964(
    u8* record);

/* @source 0x801F8BF8
 * @behavior Per-frame pass over this overlay's 0x8 work records (the
 * 0x1C-byte-stride table at 0x800E4800): it first arms the sprite draw mode
 * for the records - GetTPage(0, 1, 0x3C0, 0) yields the texture page, that page
 * is handed to SetDrawMode with the primitive published at g_PrimCursor, and
 * func_8014E5A0 queues the 0x0C-byte DR_MODE packet - then fills the 0x20-byte
 * projection scratch through func_801AFE18. It sweeps the table with an
 * unsigned byte counter, and for each record whose claim byte at offset 0 reads
 * non-zero it invokes that record's work-state handler, taken from the
 * overlay-local pointer table at 0x801FE8FC by the record's state byte at
 * offset 1, with the record in $a0, and then emits the record's sprite through
 * drawTranslucentGouraudDiscOfSixteenTrianglesAroundProjectedRecordPointScenarioScena0800_801F8964
 * with the same record. It returns 1 once any record was handled
 * and 0 when the whole table is unclaimed; it writes no memory of its own and
 * the 0x50-byte frame holds the 0x20-byte projection scratch, the outgoing
 * argument slot of SetDrawMode's fifth argument plus the saved $ra and $s0-$s3
 * of the sweep.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Matching note: the draw-mode statement has to come before the record pointer
 * and counter initializations. With them written first, the counter and pointer
 * setup is emitted ahead of SetDrawMode's argument conversion (54/57, first=
 * offset 0x0030 `lui s0,0x800e` against the original's `andi a3,v0,0xffff`);
 * moving only those two statements after it measured 57/57 at the unchanged
 * 228 bytes. Declaring the GetTPage result as u16, s32 or u32 measured
 * identically, so the page local's width is not what places the conversion.
 */
u32 tickAndRenderActiveWorkRecordsScenarioScena0800_801F8BF8(void) {
  u8 scratch[0x20];
  u8* record;
  u8 index;
  u16 tpage;
  u32 active;

  active = 0;
  tpage = GetTPage(0, 1, 0x3C0, 0);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, tpage, 0);
  record = PSX_PTR(u8, 0x800E4800u);
  index = 0u;
  func_8014E5A0(1, 0xC);
  func_801AFE18(scratch);
  do {
    if (*record != 0) {
      D_801FE8FC[record[1]](record);
      active = 1;
      drawTranslucentGouraudDiscOfSixteenTrianglesAroundProjectedRecordPointScenarioScena0800_801F8964(
          record);
    }
    index += 1u;
    record += 0x1Cu;
  } while (index < 0x8u);

  return active;
}
