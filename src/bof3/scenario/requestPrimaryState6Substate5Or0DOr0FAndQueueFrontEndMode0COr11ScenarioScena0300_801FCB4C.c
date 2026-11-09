#include "bof3/bof3.h"

extern u32 D_8014686C;
extern u8 D_80146865;
extern u8 D_80146874;
extern u8 D_80146875;
extern u8 D_80143F03;

void func_8015C088(void);
s32 func_8015B5D4(u32 arg0, s32 arg1);
s32 queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0300_801FCC9C(s32 arg0);

/* @source 0x801FCB4C
 * @behavior Route 0x63 handler of this overlay: the dispatcher at 0x801FC704
 * reaches it through its 0x63 case, forwarding both incoming packed record
 * words unchanged, so the body saves $a0 in $s0 and $a1 in $s1 before any call
 * and never writes either. It first waits through the shared two-stage helper
 * func_8015B5D4(D_8014686C, 0x1B).
 * When that reports zero it requires the front-side byte D_80143F03 to read 2,
 * the first word to equal 0xA8000 and the second word's high half to fall in
 * the inclusive range 0x24..0x25 (the `addiu -0x24` / `sltiu 2` pair), returns
 * 0 otherwise, and on that path sets the shared progress byte D_80146865 to 1,
 * requests primary scene state 6 with sub-state 5 (D_80146874 = 6,
 * D_80146875 = 5) and queues front-end mode 0x0C.
 * When the first wait reports nonzero it instead requires D_80143F03 to read 3,
 * the second word to equal 0xB8000 and the first word's high half to fall in
 * 5..6 (`addiu -0x5` / `sltiu 2`), returns 0 otherwise, and then waits again
 * through func_8015B5D4(D_8014686C, 0x1C): a nonzero answer requests state 6
 * with sub-state 0x0D and queues front-end mode 0x11, while a zero answer runs
 * the shared front-end startup helper func_8015C088, requests state 6 with
 * sub-state 0x0F and returns 1 directly without queueing a mode.
 * Both queued paths return the overlay tail's result sign-extended from its low
 * byte (sll/sra 24), and the compiler merges the two calls at 0x801FCC9C into
 * one block whose mode argument is selected on the taking path. Every failure
 * returns 0. The 0x20-byte frame
 * keeps $s0, $s1 and $ra across the calls, the D_8014686C pointer load is
 * scheduled between the second saved-argument copy and the $ra store, and the
 * constants of the next test land in the delay slots of the calls' result
 * branches. No image word of this target holds 0x801FCB4C; the dispatcher's
 * 0x63 case is its only reference.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 requestPrimaryState6Substate5Or0DOr0FAndQueueFrontEndMode0COr11ScenarioScena0300_801FCB4C(s32 arg0, s32 arg1) {
  if (func_8015B5D4(D_8014686C, 0x1B) == 0) {
    if (D_80143F03 != 2) {
      return 0;
    }

    if (arg0 != 0xA8000) {
      return 0;
    }

    if ((u32)((arg1 >> 16) - 0x24) < 2u) {
      D_80146865 = 1;
      D_80146874 = 6;
      D_80146875 = 5;
      return (s8)queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0300_801FCC9C(0xC);
    }

    return 0;
  }

  if (D_80143F03 != 3) {
    return 0;
  }

  if (arg1 != 0xB8000) {
    return 0;
  }

  if ((u32)((arg0 >> 16) - 5) < 2u) {
    if (func_8015B5D4(D_8014686C, 0x1C) == 0) {
      func_8015C088();
      D_80146875 = 0xF;
      D_80146874 = 6;
      return 1;
    }

    D_80146875 = 0xD;
    D_80146874 = 6;
    return (s8)queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0300_801FCC9C(0x11);
  }

  return 0;
}
