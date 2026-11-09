#include "bof3/bof3.h"

extern u32 D_8014686C;
extern u8 D_80146874;

s32 func_8015B5D4(u32 arg0, s32 arg1);

/* @source 0x801FC7EC
 * @behavior Route 0x33 handler of this overlay: the route dispatcher at
 * 0x801FC704 reaches it through its only jal reference (0x801FC7AC),
 * forwarding both incoming packed record words unchanged, so the body saves
 * $a0 and $a1 before any call and never writes either. It first queries the
 * shared scenario bank D_8014686C through func_8015B5D4(D_8014686C, 1) and
 * returns 0 when that reports nonzero. It then queries the same bank with
 * index 0; when that reports zero it requires the first packed word's high
 * half to fall in 2..4 (sra 16 then `addiu -2` / `sltiu 3`) and the second
 * word to equal 0x60000, and when it reports nonzero it queries index 0 once
 * more, returns 0 if that second answer is zero, and otherwise requires the
 * high half to fall in 3..4 (sltiu 2) and the second word to equal 0x40000.
 * The matched arm only picks the constant 2 or 1 for the state request, and
 * both arms share the single store of that constant into the shared
 * handler-index byte D_80146874, which the overlay primary-state table is
 * indexed by. Every path returns 0, so the function's result is always zero
 * despite the dispatcher's (s8) truncation. The 0x20-byte frame keeps $s0
 * (the first word), $s1 (the second word) and $ra across the up to three
 * calls, and the D_8014686C word load is scheduled between the second
 * saved-argument copy and the $ra store, as in this overlay's exact route
 * helpers.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 requestPrimaryState1Or2OnPackedRecordMatchScenarioScena0300_801FC7EC(s32 arg0, s32 arg1) {
  s32 next;

  if (func_8015B5D4(D_8014686C, 1) != 0) {
    return 0;
  }

  if (func_8015B5D4(D_8014686C, 0) == 0) {
    if ((u32)((arg0 >> 16) - 2) >= 3u) {
      return 0;
    }

    if (arg1 != 0x60000) {
      return 0;
    }

    next = 2;
  } else {
    if (func_8015B5D4(D_8014686C, 0) == 0) {
      return 0;
    }

    if ((u32)((arg0 >> 16) - 3) >= 2u) {
      return 0;
    }

    if (arg1 != 0x40000) {
      return 0;
    }

    next = 1;
  }

  D_80146874 = next;
  return 0;
}
