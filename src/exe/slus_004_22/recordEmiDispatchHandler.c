#include "internal.h"

extern u32         D_80146458;
extern u32         D_8014646C;
extern s8 D_80146489;
extern s8 D_801464A0[];

/* @behavior records the current EMI dispatch handler for the active ring slot,
 * marks that slot active, and advances the loader step.
 * @source 0x80162618
 */
void recordEmiDispatchHandler(void) {
  s8*  active_slot;
  u32* dispatches;

  active_slot = &D_80146489;
  dispatches = (u32*)(active_slot + 0x2f);
  dispatches[*active_slot] = D_80146458;
  /* The original reloads the slot index with a plain lb after the dispatch
   * store (memory-access ordering). A volatile pointee would emit the
   * lbu/sll/sra narrow-load quirk instead; keep this barrier. */
  barrier();
  D_801464A0[*active_slot] = 1;

  if (D_8014646C == 0) {
    D_801464A0[*active_slot] = 2;
  }

  D_8014646C += 1;
}
