#include "bof3/battle/battle03_internal.h"

/* Views of the two 0xA4-byte field-record regions this rebuild copies into each
 * local work record. Both regions start at odd byte offsets inside the field
 * record, so both views keep alignment 1: the original moves the 0x20-byte band
 * with lwl/lwr pairs and the six-byte identity run one byte at a time. */
typedef struct Battle03FieldBand {
  u8 bytes[0x20];
} Battle03FieldBand;

/* @source 0x801E0B64
 * @behavior Rebuilds the field record selected by byte +0x13C of the published
 *   local work record D_80146250 through func_80164A44. Then, for all
 *   localBattlerCount D_80145E90 local work records, copies the six-byte
 *   identity run at +0x0E and the 0x20-byte position/colour band at +0x1C of
 *   that record's field record (selected by the D_80145FCC byte at the record's
 *   own offset) into the record's +0x82 bytes and +0xB0 band. It then calls
 *   func_800AAA74, mirrors every +0xB0 band into the parallel band 0x20 bytes
 *   earlier at +0x90 (D_80145F20), calls func_800A4458 and closes with one
 *   func_800A9BD8 refresh per battler index.
 * @status exact
 * @match 100.00
 * @residual none
 */
void rebuildLocalWorkBandsFromFieldRecords(void) {
  u8                                index;
  u8                                field_index;
  volatile u8*                      count;
  volatile u8*                      band_count;
  u8*                               templates;
  u8*                               work_base;
  u8*                               src;
  u8*                               dst;

  field_index = ((u8*)D_80146250)[0x13c];
  func_80164A44((u32)((u8*)D_80144968 + field_index * 0xa4));

  count = &localBattlerCount;
  index = 0;
  if (*count != 0) {
    do {
      s32 work_offset;

      work_base = (u8*)D_80145E90 + 0x74;
      templates = (u8*)D_80144968;
      work_offset = (s32)index * 0x140;
      field_index = D_80145FCC[work_offset];
      src = templates + field_index * 0xa4;
      dst = (u8*)(work_offset + work_base);
      dst[0x0e] = src[0x0e];
      dst[0x0f] = src[0x0f];
      dst[0x10] = src[0x10];
      dst[0x11] = src[0x11];
      dst[0x12] = src[0x12];
      dst[0x13] = src[0x13];
      *(Battle03FieldBand*)&dst[0x3c] = *(Battle03FieldBand*)&src[0x1c];
      index += 1;
    } while (index < *count);
  }

  func_800AAA74();

  /* Keep the real reset ahead of the volatile count read so it can fill the
   * preceding call's delay slot. */
  do {
    index = 0;
  } while (0);
  band_count = &localBattlerCount;
  if (*band_count != 0) {
    do {
      s32 band_offset;

      band_offset = (s32)index * 0x140;
      *(Battle03FieldBand*)(band_offset + D_80145F20) =
          *(Battle03FieldBand*)(band_offset + D_80145F20 + 0x20);
      index += 1;
    } while (index < *band_count);
  }

  func_800A4458();

  /* This reset has the same lifetime boundary as the band-loop reset. */
  do {
    index = 0;
  } while (0);
  if (localBattlerCount != 0) {
    do {
      func_800A9BD8(index);
      index += 1;
    } while (index < localBattlerCount);
  }
}
