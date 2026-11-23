#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. The original materialises the cell with
 * its own address load for each of the two access groups this body makes, so
 * the source names the cell and reads it twice instead of caching one pointer
 * across the call.
 * @source 0x1F800044 @kind data
 */
extern u8* area05113WorkCursor_1F800044;
/* Work-area pointer cell of the main executable. This step reads the record
 * that sits 0x5F0 bytes before the record the cell currently points at.
 * @source 0x80146884 @kind unknown
 */
extern u8* D_80146884;

/* Shared distance helper of the main executable: it takes the signed 16.16
 * position word pair of a record and answers their planar distance, which this
 * step stores as a halfword.
 * @source 0x8015477C
 */
extern s32 func_8015477C(s32 coord_x, s32 coord_y);

/* @source 0x801F2C7C
 * @behavior Copies the position pair of the work-area record that sits 0x5F0
 *           bytes before the record published through the pointer cell
 *           0x80146884 into the work record reached through the scratchpad
 *           pointer cell 0x1F800044: the source word 0x34 goes to the work
 *           record word 0x34 and the source word 0x38 to the work record word
 *           0x38. It then re-reads work record word 0x34 and hands that word
 *           together with the copied second word to the shared distance helper
 *           func_8015477C, re-reads the scratchpad cell because the helper may
 *           republish it, and stores the returned distance minus 0x40 in the
 *           work record halfword 0x3E. The 0x18-byte frame only keeps $ra
 *           across the call. Takes no arguments and returns nothing; no jal
 *           word and no data word of this payload names it, so it is a
 *           script-invoked step and its argument roles stay structural.
 *
 * Name evidence: the helper, the scratchpad work-record cell, the work-record
 *           word 0x34 as the X coordinate, word 0x38 as the Y coordinate and
 *           halfword 0x3E as a stored distance are the roles
 *           include/bof3/ui/game00_internal.h records for these offsets; the
 *           source record's identity 0x5F0 bytes before the cell's record is
 *           not established, so that operand stays an address-anchored offset.
 *
 * Representation notes: the original builds the source base in one register
 *           with `addiu` and reaches both copied words as 0x34/0x38
 *           displacements of it, so the subtraction takes its own statement on
 *           the pointer local. Spelling the whole base as one expression
 *           (`source = D_80146884 - 0x5F0;`) folded the offset into the two
 *           loads as -0x5BC/-0x5B8 displacements of the raw cell value and
 *           measured 11/23 instructions at 80 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyAreaStoreDistance(
    void) {
  u8* work;
  u8* source;
  u8* target;
  s32 y;
  s32 distance;

  source = D_80146884;
  source -= 0x5F0;
  work = area05113WorkCursor_1F800044;
  *(s32*)(work + 0x34) = *(s32*)(source + 0x34);
  y = *(s32*)(source + 0x38);
  *(s32*)(work + 0x38) = y;
  distance = func_8015477C(*(s32*)(work + 0x34), y) - 0x40;
  target = area05113WorkCursor_1F800044;
  *(s16*)(target + 0x3E) = distance;
}
