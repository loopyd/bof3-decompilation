#include "bof3/bof3.h"

/* The kind byte +0x79 of the first 0x140-stride local work record of the run at
 * 0x80145E90, and the mode byte the battle hooks of this overlay publish. */
extern u8 D_80145F09[]; /* @source 0x80145F09 @kind unknown */
extern u8 D_80146255;   /* @source 0x80146255 @kind unknown */

/* @source 0x800C2A40
 * @behavior Reads the unsigned kind byte of the first local work record of the
 * battle work run (flat symbol 0x80145F09, the +0x79 byte of the 0x140-stride
 * run at 0x80145E90) and publishes one mode byte for it: kind 0 stores 0x2C, kind 1
 * stores 0x2D, kind 5 stores 0x2E and kind 6 stores 0x2F into the shared battle
 * mode byte 0x80146255. Any other kind - 2, 3, 4 and 7 upward - stores nothing,
 * so the four consecutive mode values 0x2C..0x2F cover exactly the kinds this
 * overlay animates. Every arm ends at the one store site, which the written
 * per-arm stores cross-jump into, and the two arms that store the last two
 * constants reuse one tail. Takes no arguments, returns nothing, and calls
 * nothing; its callers are func_800C2258, func_800C253C and func_800C2820, which
 * set the action byte 0x80146875 first and run initModeTuple530 (0x801DDF28)
 * after it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectModeByteByLocalWorkKindBossBoss01816_800C2A40(void) {
  switch (D_80145F09[0]) {
  case 0:
    D_80146255 = 0x2C;
    break;
  case 1:
    D_80146255 = 0x2D;
    break;
  case 5:
    D_80146255 = 0x2E;
    break;
  case 6:
    D_80146255 = 0x2F;
    break;
  }
}
