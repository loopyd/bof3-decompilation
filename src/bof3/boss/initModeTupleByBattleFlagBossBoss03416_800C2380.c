#include "bof3/bof3.h"

extern u8 D_80146328; /* @source 0x80146328 @kind unknown */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */

void initModeTuple510(void); /* @source 0x801DDED8 */
void initModeTuple520(void); /* @source 0x801DDF00 */

/* @source 0x800C2380
 * @behavior Overlay mode-transition handler. It reads the shared battle flag
 * byte D_80146328 and, while that byte's 0x2 bit is set, stores 5 into the
 * shared mode byte D_80146875 and runs initModeTuple510, the service that
 * seeds the three-byte battle mode tuple with 5, 1 and 0; when the bit is
 * clear it runs initModeTuple520 instead, which seeds the same tuple with 5, 2
 * and 0. It takes no arguments and returns nothing. No word of this payload
 * holds this function's address and no `jal` reaches it, so the main program
 * reaches it through the overlay entry point. It is the mode-transition member
 * of the handler trio this archive family publishes into the shared battle hook
 * slots, and it shares that body's shape with the sibling overlays, which
 * differ only in the constant they store into the shared mode byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initModeTupleByBattleFlagBossBoss03416_800C2380(void) {
  if (D_80146328 & 0x2) {
    D_80146875 = 5;
    initModeTuple510();
  } else {
    initModeTuple520();
  }
}
