#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern volatile u8* area06513WorkCursor_1F800044;
/* @source 0x8014496D @kind unknown */
extern u8 D_8014496D;
/* @source 0x80146871 @kind unknown */
extern u8 D_80146871;
/* @source 0x80104000 @kind unknown */
extern u8 D_80104000;
/* @source 0x8014931C @kind unknown */
extern u32 D_8014931C;
/* @source 0x801F4DE0 @kind data */
extern const u8 D_801F4DE0[][4];

/* @source 0x80196070 */
extern void func_80196070(void);
/* @source 0x8019A194 */
extern u8 func_8019A194(void);
/* @source 0x8014DD3C */
extern void func_8014DD3C(u16 entry);
/* @source 0x8014D6B8 */
extern void func_8014D6B8(u32 flag);

/* @source 0x801F4648
 * @behavior If byte 0x8014496D is 9 or bit 0 of byte 0x80146871 is set,
 * calls 0x80196070 and returns. Otherwise calls 0x8019A194, applies lookup
 * entry 0x205, clears work bytes 0x48, 0x24, 0x2A and 0x5D through 0x5F,
 * then writes 0xA0 to the map byte selected by the column and row in table
 * 0x801F4DE0 indexed by work byte 0x0B. Calls 0x8014D6B8 with 0 and sets
 * work state byte 1 to 1. The work pointer is reloaded for each store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeWorkAndStampMapA0UnlessGatedWorld01Area06513_801F4648(void) {
  u8 index;

  if (D_8014496D == 9 || (D_80146871 & 1) != 0) {
    func_80196070();
    return;
  }

  func_8019A194();
  func_8014DD3C(0x205);
  area06513WorkCursor_1F800044[0x48] = 0;
  FIELD_REF(u8, area06513WorkCursor_1F800044, 0x24u) = 0;
  FIELD_REF(u8, area06513WorkCursor_1F800044, 0x2Au) = 0;
  FIELD_REF(u8, area06513WorkCursor_1F800044, 0x5Du) = 0;
  FIELD_REF(u8, area06513WorkCursor_1F800044, 0x5Eu) = 0;
  FIELD_REF(u8, area06513WorkCursor_1F800044, 0x5Fu) = 0;

  index = area06513WorkCursor_1F800044[0x0B];
  *(u8*)(D_8014931C + D_801F4DE0[index][1] * D_80104000 +
         D_801F4DE0[index][0]) = 0xA0;

  func_8014D6B8(0);
  FIELD_REF(u8, area06513WorkCursor_1F800044, 0x01u) = 1;
}
