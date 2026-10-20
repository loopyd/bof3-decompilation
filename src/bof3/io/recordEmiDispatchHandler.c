#include "bof3/core/slus_internal.h"

extern u32 D_80146458;
extern u32 D_8014646C;
extern s8  D_80146489;
extern u8  D_801464A0[];

/* @behavior records the current EMI dispatch handler for the active ring slot,
 * marks that slot active, and advances the loader step.
 * @source 0x80162618
 * @status exact
 * @match 100.00
 * @residual none
 */
void recordEmiDispatchHandler(void) {
  s8* slot;
  u32* table;

  slot = &D_80146489;
  table = (u32*)(slot + 0x2f);
  /* The loader slot table is written asynchronously by the CD callback path;
   * the reviewed sibling lift startEmiEntryTransfer (0x80162898) stores to the
   * same table through the same volatile view. */
  *(volatile u32*)&table[*slot] = D_80146458;
  D_801464A0[*slot] = 1;
  if (D_8014646C == 0) {
    D_801464A0[*slot] = 2;
  }
  D_8014646C += 1;
}
