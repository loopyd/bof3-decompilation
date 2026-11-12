#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* D_1F800044;
/* @source 0x8014496D @kind unknown */
extern u8 D_8014496D;
/* @source 0x80146871 @kind unknown */
extern u8 D_80146871;
/* @source 0x80104000 @kind unknown */
extern u8 D_80104000;
/* @source 0x8014931C @kind unknown */
extern u32 D_8014931C;
/* @source 0x801F591C @kind data */
extern const u8 D_801F591C[][4];

u8 func_8019A194(void);
void func_80196070(void);
void func_8014DD3C(u16 entry);
void func_8014D6B8(u32 flag);

/* @source 0x801F5170
 * @behavior State handler 0 of the two-word handler table 0x801F6270, the pair
 *           of code words 0x801F5170 and 0x801F52A0 that the local dispatcher
 *           0x801F512C reads by work byte 0x01. When the shared byte at
 *           0x8014496D reads 9, or bit 0 of the shared byte at 0x80146871 is
 *           set, it calls the shared helper 0x80196070 and returns. Otherwise
 *           it calls the shared helper 0x8019A194, requests resource 0x205
 *           through the shared helper 0x8014DD3C, and clears the work bytes at
 *           offsets 0x48, 0x24, 0x2A and 0x5D through 0x5F of the record
 *           published at the scratchpad pointer cell 0x1F800044, reloading
 *           that cell for each store. It then stamps 0xA0 into the map byte at
 *           the column and row held by the four-byte record at 0x801F591C that
 *           work byte 0x0B selects (map base word 0x8014931C plus row times the
 *           width byte 0x80104000 plus column), calls the shared helper
 *           0x8014D6B8 with 0 and finally sets work byte 0x01 to 1. It takes no
 *           arguments, saves only the return address at 0x10 of its 0x18-byte
 *           frame and returns nothing. The 0x130 payload bytes matched the
 *           exact initializeWorkAndStampMapA0UnlessGatedWorld01Area06513_801F4648
 *           in emi/world01/area065/13 at every byte except six at three words:
 *           the `j` word at span offset 0x38, whose target is this span's own
 *           epilogue and therefore shifts by exactly the 0xB28 between the two
 *           spans, and the two low halves at offsets 0x0D4 and 0x0E8 that carry
 *           each overlay's own four-byte lookup table (0x801F591C here,
 *           0x801F4DE0 there), so this body takes that family's statement order
 *           with this target's own table and literals.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeWorkAndStampMapA0UnlessGatedWorld02Area08813_801F5170(void) {
  u8 index;

  if (D_8014496D == 9 || (D_80146871 & 1) != 0) {
    func_80196070();
    return;
  }

  func_8019A194();
  func_8014DD3C(0x205);
  D_1F800044[0x48] = 0;
  FIELD_REF(u8, D_1F800044, 0x24u) = 0;
  FIELD_REF(u8, D_1F800044, 0x2Au) = 0;
  FIELD_REF(u8, D_1F800044, 0x5Du) = 0;
  FIELD_REF(u8, D_1F800044, 0x5Eu) = 0;
  FIELD_REF(u8, D_1F800044, 0x5Fu) = 0;

  index = D_1F800044[0x0B];
  *(u8*)(D_8014931C + D_801F591C[index][1] * D_80104000 +
         D_801F591C[index][0]) = 0xA0;

  func_8014D6B8(0);
  FIELD_REF(u8, D_1F800044, 0x01u) = 1;
}
