#include "internal.h"

typedef void (*EmiLoaderCallback)(void);

typedef struct EmiCallbackGroup {
  u32 values[4];
} EmiCallbackGroup;

extern s8        D_8014648A;
extern const u32 D_80149C3C[];

/* @behavior snapshots the target-local EMI callback table and invokes the
 * callback selected by the current loader mode.
 * @source 0x80162d18 func_80162d18
 */
void func_80162d18(void) {
  EmiCallbackGroup*       dst;
  const EmiCallbackGroup* src;
  const EmiCallbackGroup* end;
  u32                     callbacks[9];

  dst = (EmiCallbackGroup*)callbacks;
  src = (const EmiCallbackGroup*)D_80149C3C;
  end = src + 2;
  do {
    *dst = *src;
    src++;
    dst++;
  } while (src != end);
  *(vu32*)dst = src->values[0];

  ((EmiLoaderCallback)callbacks[D_8014648A])();
}
