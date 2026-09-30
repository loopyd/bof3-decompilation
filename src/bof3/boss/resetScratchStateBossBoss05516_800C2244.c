#include "bof3/bof3.h"

void func_801E567C(void);

/* @source 0x800C2244
 * @behavior Overlay handler referenced by the function-pointer table word at
 * 0x800C35E8 (inside the func_800C34D4 boundary): calls the engine scratch-state
 * reset at 0x801E567C, then repeats its two stores on the scratchpad work object
 * published by the pointer cell at 0x1F800044 - byte +1 = 2, byte +2 = 0.
 * The original materialises the cell address twice (`lui`/`lw` pair at
 * 0x800C2254 and again at 0x800C2264, each using the load destination as base):
 * the cell is a plain fixed-address pointer, and the second read is a fresh one
 * because the store through the loaded pointer may alias it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetScratchStateBossBoss05516_800C2244(void) {
  u8** scratch = SPAD_PTR_TABLE(u8);

  func_801E567C();
  scratch[0x11][1] = 2;
  scratch[0x11][2] = 0;
}
