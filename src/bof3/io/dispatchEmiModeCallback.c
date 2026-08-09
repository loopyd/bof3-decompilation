#include "bof3/core/slus_internal.h"

/* @behavior snapshots the target-local EMI callback table and invokes the
 * callback selected by the current loader mode.
 * @source 0x80162D18
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */

typedef void (*EmiLoaderCallback)(void);

typedef struct EmiCallbackGroup {
  u32 values[4];
} EmiCallbackGroup;

extern s8        D_8014648A;
/* @source 0x80149C3C @kind table */
extern const u32 emiLoaderCallbackTable[];

void dispatchEmiModeCallback(void) {
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
