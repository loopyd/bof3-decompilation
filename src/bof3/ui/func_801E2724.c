#include "bof3/ui/shop00_internal.h"

/* @source 0x801E2724
 * @behavior initializes two shop UI panel records.
 * @status review-pending
 * @match 53.85
 * @residual without the unsupported barrier, output is 156 bytes; base/constants are optimized away or allocated late and the snapshot load remains after the first stores.
 */
typedef struct ShopPanelEndpoint {
  u8 enabled;
  u8 pad_01[0x23];
} ShopPanelEndpoint;

void func_801E2724(void) {
  ShopPanelEndpoint* records;
  u8 seven;
  u8 snapshot;
  u8 two;
  u8 one;

  records = (ShopPanelEndpoint*)D_80148378;
  seven = 7;
  two = 2;
  D_8014837A = 16;
  D_8014837C = 320;
  D_8014837E = 63;
  snapshot = D_80148361;
  D_80148379 = seven;
  D_8014837B = two;
  D_80148382 = snapshot;
  one = 1;
  records[0].enabled = one;
  D_8014839E = 17;
  D_801483A0 = 180;
  D_8014839D = seven;
  D_8014839F = two;
  D_801483A2 = 240;
  records[1].enabled = one;
}
