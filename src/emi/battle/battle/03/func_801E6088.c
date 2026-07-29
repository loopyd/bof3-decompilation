#include "internal.h"

/* @behavior decrements the queued slot's timer and advances its selector on wrap. */
void func_801E6088(void) {
  if (D_801EC2E0->unk_09-- == 0) {
    D_801EC2E0->unk_01++;
    D_801EC2E0->unk_09 = 4;
  }
}
