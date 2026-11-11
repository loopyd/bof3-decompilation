#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell published by the front end. The original
 * re-reads it once per group: before each of the three leading byte stores,
 * before the two seeded words, before the record byte 0x06 test, before the
 * 0x0B store, again for the byte 0x01 advance and once more per table lookup,
 * so this body addresses it by name at every access.
 * @source 0x1F800044 @kind unknown
 */
extern u8* D_1F800044;
/* Shared position word of the main executable copied into record offset 0x34.
 * @source 0x80145EC4 @kind unknown
 */
extern u32 D_80145EC4;
/* Shared position word of the main executable copied into record offset 0x38.
 * @source 0x80145EC8 @kind unknown
 */
extern u32 D_80145EC8;
/* Stride-4 signed halfword pair table of this payload's own textbin tail: the
 * two halfwords of row k sit at 0x801F54E4 + 4k and + 4k + 2, and facing byte
 * 0x08 of the work record selects the row. Column 0 seeds record word 0x0C and
 * column 1 seeds word 0x10, and the same
 * rows carry the two step terms the body subtracts from the record halfwords
 * 0x36 and 0x3A, first as the signed three-bit field and then as the signed
 * nine-bit field of each halfword.
 * @source 0x801F54E4 @kind table
 */
extern u16 D_801F54E4[][2];
/* Stride-2 byte pair table of the same tail at 0x801F54F4, indexed by the same
 * facing byte 0x08: column 0 is the id requested through 0x8014D6B8 and column
 * 1 is stored into record byte 0x2A before the 0x8014D290 call.
 * @source 0x801F54F4 @kind table
 */
extern u8 D_801F54F4[][2];

void func_8014DD3C(u16 entry);
void func_8014D6B8(u32 resource);
void func_8014D290(void);

/* @source 0x801F4264
 * @behavior Entry 1 of the three-entry byte-1 handler run based at 0x801F54D8,
 *           whose dispatcher is 0x801F40E4: the body address appears once in
 *           the payload, at word 0x801F54DC, and no `jal` in the payload names
 *           it, so it takes no arguments and answers no value. It applies entry
 *           0x46 through 0x8014DD3C, then clears record bytes 0x48 and 0x24 and
 *           sets byte 0x29 to 5, each through a fresh read of the scratchpad
 *           pointer cell 0x1F800044. It seeds record words 0x0C and 0x10 from
 *           the signed halfword pair selected by facing byte 0x08 in table
 *           0x801F54E4 and copies the shared words 0x80145EC4 and 0x80145EC8
 *           into record offsets 0x34 and 0x38. While record byte 0x06 is
 *           nonzero it subtracts the pair shifted right by 13 from halfwords
 *           0x36 and 0x3A, then raises halfword 0x36 by 2 when word 0x0C is
 *           zero and record byte 0x06 reads 1, otherwise it moves halfword 0x3A
 *           by +2 for that case and by -2 for every other byte 0x06 reading. It
 *           then subtracts the pair shifted right by 9 from both halfwords,
 *           clears record byte 0x0B and increments state byte 0x01 so the next
 *           dispatch takes entry 2 of the same run at 0x801F4494. Finally it
 *           requests the first byte of the facing-selected pair at 0x801F54F4
 *           through 0x8014D6B8, stores the second byte in record byte 0x2A and
 *           calls 0x8014D290. The 0x18-byte frame holds only the return address.
 *
 * Shape provenance: the 0x230-byte span compared byte-equal to the exact lift at
 * 0x801F422C of emi/world00/area033/13 apart from ten words - the eight `%lo`
 * immediates of the two in-payload table bases, which are this payload's own
 * data addresses, and the two `j` words of the shared tail label, whose target
 * shifts by exactly the 0x38 offset between the two spans. That comparison made
 * the reviewed sibling the statement order, and every table address and literal
 * here comes from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedFacingStepPairAndStepPositionThenAdvanceModeAndRequestResourceWorld03Area15113_801F4264(
    void) {
  u8* record;
  s16 stepped;

  func_8014DD3C(0x46);
  D_1F800044[0x48] = 0;
  D_1F800044[0x24] = 0;
  D_1F800044[0x29] = 5;

  record = D_1F800044;
  FIELD_REF(s32, record, 0x0C) = (s16)D_801F54E4[record[0x08]][0];
  FIELD_REF(s32, record, 0x10) = (s16)D_801F54E4[record[0x08]][1];
  FIELD_REF(u32, record, 0x34) = D_80145EC4;
  FIELD_REF(u32, record, 0x38) = D_80145EC8;

  if (record[0x06] != 0) {
    FIELD_REF(u16, record, 0x36) -= (s16)D_801F54E4[record[0x08]][0] >> 13;
    stepped =
        FIELD_REF(u16, record, 0x3A) - ((s16)D_801F54E4[record[0x08]][1] >> 13);
    FIELD_REF(u16, record, 0x3A) = stepped;
    if (record[0x06] == 1) {
      if (FIELD_REF(s32, record, 0x0C) == 0) {
        FIELD_REF(u16, record, 0x36) += 2;
      } else {
        FIELD_REF(u16, record, 0x3A) = stepped + 2;
      }
    } else if (FIELD_REF(s32, record, 0x0C) == 0) {
      FIELD_REF(u16, record, 0x36) -= 2;
    } else {
      FIELD_REF(u16, record, 0x3A) = stepped - 2;
    }
  }

  {
    u8* next;
    u16 half;

    record = D_1F800044;
    FIELD_REF(u16, record, 0x36) -= (s16)D_801F54E4[record[0x08]][0] >> 9;
    half = FIELD_REF(u16, record, 0x3A) - ((s16)D_801F54E4[record[0x08]][1] >> 9);
    record[0x0B] = 0;
    next = D_1F800044;
    FIELD_REF(u16, record, 0x3A) = half;
    next[0x01]++;
  }
  func_8014D6B8(D_801F54F4[D_1F800044[0x08]][0]);
  D_1F800044[0x2A] = D_801F54F4[D_1F800044[0x08]][1];
  func_8014D290();
}
