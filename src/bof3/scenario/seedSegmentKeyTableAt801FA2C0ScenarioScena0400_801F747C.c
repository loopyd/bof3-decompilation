#include "bof3/bof3.h"

/* Five key records of 0x20 bytes each: two 16.16 points per record, held as
 * three start/end word pairs at +0/+0x10, +0x4/+0x14 and +0x8/+0x18.
 * @source 0x801FA2C0 @kind table
 */
extern s32 D_801FA2C0[];

/* @source 0x801F747C
 * @behavior Seeds this overlay's five-record segment key table at 0x801FA2C0:
 *           every record starts with the same pair 0x00568000 to 0x005E8000,
 *           the second pair is 0x00118000 to 0x000F0000 in records 0 and 1 and
 *           0x00138000 to 0x00158000, 0x00160000, 0x00160000 in records 2, 3
 *           and 4, and the third pair is 0x03000000 to 0x04000000,
 *           0x02400000 to 0x02400000, 0x03000000 to 0x04000000, 0x02800000 to
 *           0x02C00000 and 0x01200000 to 0x01200000. The words at +0xC and
 *           +0x1C of each record are left untouched. Its only caller,
 *           func_801F6D44 (step 0 of the work-byte-1 handler run), calls it
 *           after it stamps the frame primitive words into the scratch work
 *           object at 0x1F800044 and before it sets work byte 1 to 1; its only
 *           reader, func_801F75BC (0x801F75BC), walks the five records by the
 *           index byte it is given, lerps each pair by that call's other byte
 *           over 16, projects the record start point and the lerped point
 *           through func_801AFF04 into the two primitive slots it fills, and
 *           stops once the index reaches five, so each record is one segment
 *           that grows from its start point. It reads nothing, makes no call
 *           and keeps no frame; the five record start words go through a
 *           pointer advanced by eight words between records while each
 *           record's other five words are stored at their own fixed addresses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedSegmentKeyTableAt801FA2C0ScenarioScena0400_801F747C(void) {
  s32* p;

  p = D_801FA2C0;
  *p = 0x568000;
  *PSX_PTR(s32, 0x801FA2C4) = 0x00118000;
  *PSX_PTR(s32, 0x801FA2C8) = 0x03000000;
  *PSX_PTR(s32, 0x801FA2D0) = 0x005E8000;
  *PSX_PTR(s32, 0x801FA2D4) = 0x000F0000;
  *PSX_PTR(s32, 0x801FA2D8) = 0x04000000;
  p += 8;
  *p = 0x568000;
  *PSX_PTR(s32, 0x801FA2E4) = 0x00118000;
  *PSX_PTR(s32, 0x801FA2E8) = 0x02400000;
  *PSX_PTR(s32, 0x801FA2F0) = 0x005E8000;
  *PSX_PTR(s32, 0x801FA2F4) = 0x000F0000;
  *PSX_PTR(s32, 0x801FA2F8) = 0x02400000;
  p += 8;
  *p = 0x568000;
  *PSX_PTR(s32, 0x801FA304) = 0x00138000;
  *PSX_PTR(s32, 0x801FA308) = 0x03000000;
  *PSX_PTR(s32, 0x801FA310) = 0x005E8000;
  *PSX_PTR(s32, 0x801FA314) = 0x00158000;
  *PSX_PTR(s32, 0x801FA318) = 0x04000000;
  p += 8;
  *p = 0x568000;
  *PSX_PTR(s32, 0x801FA324) = 0x00138000;
  *PSX_PTR(s32, 0x801FA328) = 0x02800000;
  *PSX_PTR(s32, 0x801FA330) = 0x005E8000;
  *PSX_PTR(s32, 0x801FA334) = 0x00160000;
  *PSX_PTR(s32, 0x801FA338) = 0x02C00000;
  p += 8;
  *p = 0x568000;
  *PSX_PTR(s32, 0x801FA344) = 0x00138000;
  *PSX_PTR(s32, 0x801FA348) = 0x01200000;
  *PSX_PTR(s32, 0x801FA350) = 0x005E8000;
  *PSX_PTR(s32, 0x801FA354) = 0x00160000;
  *PSX_PTR(s32, 0x801FA358) = 0x01200000;
}
