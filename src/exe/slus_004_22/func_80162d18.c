#include "internal.h"

typedef void (*EmiLoaderCallback)(void);

typedef struct EmiCallbackGroup {
  u32 values[4];
} EmiCallbackGroup;

extern s8        DAT_8014648a;
extern const u32 DAT_80149c3c[];

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
  src = (const EmiCallbackGroup*)DAT_80149c3c;
  end = src + 2;
  do {
    *dst = *src;
    src++;
    dst++;
  } while (src != end);
  callbacks[8] = src->values[0];

  ((EmiLoaderCallback)callbacks[DAT_8014648a])();
}
