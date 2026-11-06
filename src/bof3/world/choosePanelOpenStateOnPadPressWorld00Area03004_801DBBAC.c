#include "bof3/world/area03004_internal.h"

/* @source 0x801DBBAC
 * @behavior AREA030 panel step, entry 9 of the local work-record step table
 * 0x801E2268: submits the panel pair at y 78, configures the sprite at 0x80
 * with CLUT 91 and byte 6 of the active 0x98-byte work record
 * D_80146888[D_801E3208], draws that record byte together with the record byte
 * at +0x90 through func_801E1004, draws the icon row at (176, 156) through
 * func_801E1320 and the extra icon through func_801E1C7C. While the shared pad
 * mask D_80145AA8/0x50 is set it clears the extra-icon flag D_801E3214 and
 * scratch byte 0xB, then picks the panel open state: record byte 6 equal to
 * 0x16 arms byte 9 with 4 and stores 1 in byte 0xB, a failing
 * func_801650B4(0, byte 6 + 0x38, 1, 0) gate arms byte 9 with 4 and stores 2 in
 * byte 0xB, and a passing gate advances dispatch byte 3 to 0x0E; it then plays
 * cue 0x102 and increments that dispatch byte. Finally it runs func_8014D978
 * and appends the dim tile.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Three shapes carry the measurement. The record byte 6 read is one u8 local
 * consumed twice: the zero-extended comparison copy feeds the 0x16 test while
 * the original value stays in the argument register for the gate call. The
 * scratch cursor read that clears byte 0xB is its own pointer local, because a
 * single cursor variable for the whole pad block allocates that early group to
 * the register the later arms need. The 0x0D dispatch store reads the
 * fixed-address scratch view rather than that pointer local so the constant load
 * stays in the delay slot of the gate branch, ahead of the cursor load.
 */
void choosePanelOpenStateOnPadPressWorld00Area03004_801DBBAC(void) {
  u8* work;
  u8* cursor;
  u8 recordId;

  submitPanelPair(0, 78);
  configureSpriteClut(0x80, 91, D_80146888[D_801E3208].unk_02[4]);
  func_801E1004(D_80146888[D_801E3208].unk_02[4],
                D_80146888[D_801E3208].unk_60[0x30], 0);
  func_801E1320(176, 156);
  func_801E1C7C();
  if ((D_80145AA8 & 0x50) != 0) {
    cursor = D_1F800044;
    D_801E3214 = 0;
    cursor[0xB] = 0;
    recordId = D_80146888[D_801E3208].unk_02[4];
    if (recordId == 0x16) {
      work = D_1F800044;
      work[9] = 4;
      work = D_1F800044;
      work[0xB] = 1;
    } else if (func_801650B4(0, recordId + 0x38, 1, 0) == 0) {
      work = D_1F800044;
      work[9] = 4;
      work = D_1F800044;
      work[0xB] = 2;
    } else {
      WORLD00_AREA030_SCRATCH_PTR[3] = 0x0D;
    }
    func_8015DF18(0x102);
    work = D_1F800044;
    work[3]++;
  }
  func_8014D978();
  appendDimTile();
}
