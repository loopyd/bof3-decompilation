#include "internal.h"

/* @behavior Selects the local work array for selectors below three and the alternate region otherwise. */
u8* func_801DD760(u8 arg0) {
  if (arg0 < 3u) {
    return (u8*)&D_80145E90[arg0];
  }

  return D_801EB2E8 + ((arg0 * 36u - arg0) * 8u);
}
