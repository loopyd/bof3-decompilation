#include "bof3/bof3.h"

extern u8 D_80145029; /* @source 0x80145029 @kind unknown */
extern u8 D_80146255; /* @source 0x80146255 @kind unknown */
extern u8 D_80146328; /* @source 0x80146328 @kind unknown */
extern u8 D_80146874; /* @source 0x80146874 @kind unknown */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */
extern u8 D_8014832E; /* @source 0x8014832E @kind unknown */

/* Shared mode-tuple service of the main program, reached through the target map
 * row `initModeTuple530 = 0x801DDF28` this payload adds next to the raw
 * undefined-function row; the in-repo lift of 0x801DDF28 seeds the three-byte
 * tuple at 0x801462E0 with 5, 3 and 0. The whole boss archive family runs it at
 * the end of its battle-flag state switch. */
void initModeTuple530(void); /* @source 0x801DDF28 */

/* @source 0x800C1CB4
 * @behavior Battle-flag state switch of this overlay and first member of the
 * hook trio its installer writes. It takes the address of the shared battle
 * flag byte 0x80146328 into one local pointer, loads that byte once and tests
 * bit 0x1; while the bit is set it stores the mode byte 6 into 0x80146255,
 * stores 6 into the shared mode byte 0x80146874, stores 0x32 into the shared
 * step byte 0x80146875 and runs initModeTuple530 (0x801DDF28), which seeds the
 * three-byte battle mode tuple with 5, 3 and 0; when the bit is clear it stores
 * the mode byte 2 into 0x80146255, clears the shared effect-bank byte
 * 0x8014832E, writes the value it already loaded with bit 0x8 raised back to
 * 0x80146328, advances 0x80146875 by one through a second local pointer, stores
 * 4 into the front-end selection index 0x80145029 and then runs the same
 * initModeTuple530. It reads no other state, takes no arguments, returns
 * nothing, and its 0x18-byte frame keeps $ra at 0x10 across the tail call. The
 * installer at 0x800C1A0C materializes this function's address into the shared
 * battle-hook slot 0x801463A4; no word of this payload holds that address and
 * no `jal` reaches it, so the main program reaches it through that slot. Same hook slot and condition shape as the
 * exact siblings initModeTuple510ByBattleFlagAndSetScenarioProgressBossBoss01216_800C1BA8
 * and initModeTuple510Or520ByFlagBitTwoBossBoss01416_800C1AD4, which load the
 * same flag byte once and branch on a bit of it before their mode-tuple call;
 * this body differs in the bit it tests, in the mode/step values it writes and
 * in carrying the effect-bank clear and the selection-index store on the clear
 * path.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initModeTuple530ByBattleFlagBitZeroAndClearEffectBankBossBoss00716_800C1CB4(
    void) {
  u8* flags;
  u8* step;
  u8 value;

  flags = &D_80146328;
  value = *flags;
  if (value & 0x1) {
    D_80146255 = 6;
    D_80146874 = 6;
    D_80146875 = 0x32;
  } else {
    D_80146255 = 2;
    D_8014832E = 0;
    *flags = value | 0x8;
    step = &D_80146875;
    *step = *step + 1;
    D_80145029 = 4;
  }
  initModeTuple530();
}
