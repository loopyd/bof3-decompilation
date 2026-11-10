#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* D_1F800044;
/* @source 0x80145EC4 @kind unknown */
extern u32 D_80145EC4;
/* @source 0x80145EC8 @kind unknown */
extern u32 D_80145EC8;
/* @source 0x801F501C @kind table */
extern u16 D_801F501C[][2];
/* @source 0x801F502C @kind table */
extern u8 D_801F502C[][2];

extern void func_8014DD3C(u16 entry);
extern void func_8014D6B8(u32 resource);
extern void func_8014D290(void);

/* @source 0x801F422C
 * @behavior Mode-1 handler of the three-entry table at 0x801F5010, indexed
 * by work byte 0x01 through the dispatcher at 0x801F40AC. Applies lookup entry
 * 0x46 through 0x8014DD3C, clears work bytes 0x48 and 0x24 and sets byte 0x29
 * to 5. Seeds words 0x0C and 0x10 from the signed halfword pair selected by
 * facing byte 0x08 in table 0x801F501C, and copies shared position words
 * 0x80145EC4 and 0x80145EC8 into work offsets 0x34 and 0x38. When byte 0x06
 * is nonzero, subtracts the pair shifted right by 13 from halfwords 0x36 and
 * 0x3A, then adjusts halfword 0x36 if word 0x0C is zero, otherwise halfword
 * 0x3A, by +2 for byte 0x06 equal to 1 or -2 otherwise. Subtracts the pair
 * shifted right by 9 from both halfwords, clears byte 0x0B and advances mode
 * byte 0x01. Requests the first byte of the facing-selected pair at 0x801F502C
 * through 0x8014D6B8, stores the second byte in work byte 0x2A and calls
 * 0x8014D290. Uses the record published at scratchpad pointer cell 0x1F800044.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedFacingStepPairAndStepPositionThenAdvanceModeAndRequestResourceWorld00Area03313_801F422C(
    void) {
  u8* record;
  s16 stepped;

  func_8014DD3C(0x46);
  D_1F800044[0x48] = 0;
  D_1F800044[0x24] = 0;
  D_1F800044[0x29] = 5;

  record = D_1F800044;
  FIELD_REF(s32, record, 0x0C) = (s16)D_801F501C[record[0x08]][0];
  FIELD_REF(s32, record, 0x10) = (s16)D_801F501C[record[0x08]][1];
  FIELD_REF(u32, record, 0x34) = D_80145EC4;
  FIELD_REF(u32, record, 0x38) = D_80145EC8;

  if (record[0x06] != 0) {
    FIELD_REF(u16, record, 0x36) -= (s16)D_801F501C[record[0x08]][0] >> 13;
    stepped =
        FIELD_REF(u16, record, 0x3A) - ((s16)D_801F501C[record[0x08]][1] >> 13);
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
    FIELD_REF(u16, record, 0x36) -= (s16)D_801F501C[record[0x08]][0] >> 9;
    half = FIELD_REF(u16, record, 0x3A) - ((s16)D_801F501C[record[0x08]][1] >> 9);
    record[0x0B] = 0;
    next = D_1F800044;
    FIELD_REF(u16, record, 0x3A) = half;
    next[0x01]++;
  }
  func_8014D6B8(D_801F502C[D_1F800044[0x08]][0]);
  D_1F800044[0x2A] = D_801F502C[D_1F800044[0x08]][1];
  func_8014D290();
}
