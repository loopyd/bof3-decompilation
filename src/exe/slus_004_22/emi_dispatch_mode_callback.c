#include "internal.h"

typedef void (*EmiLoaderCallback)(void);

typedef struct EmiCallbackGroup {
  u32 values[4];
} EmiCallbackGroup;

extern s8        D_8014648A;
extern const u32 emiLoaderCallbackTable[]; /* @kind: table */

/* @behavior snapshots the target-local EMI callback table and invokes the
 * callback selected by the current loader mode.
 * @source 0x80162D18
 */
void emi_dispatch_mode_callback(void) {
  EmiCallbackGroup*       dst;
  const EmiCallbackGroup* src;
  const EmiCallbackGroup* end;
  u32                     callbacks[9];

  dst = (EmiCallbackGroup*)callbacks;
  src = (const EmiCallbackGroup*)emiLoaderCallbackTable;
  end = src + 2;
  do {
    *dst = *src;
    src++;
    dst++;
  } while (src != end);
  *(volatile u32*)dst = src->values[0];

  ((EmiLoaderCallback)callbacks[D_8014648A])();
}
