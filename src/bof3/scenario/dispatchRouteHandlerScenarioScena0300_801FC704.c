#include "bof3/bof3.h"

extern u16 D_80143F00;

s32 func_801FC7EC(s32 arg0, s32 arg1);
s32 func_801FC8AC(s32 arg0, s32 arg1);
s32 requestPrimaryState4Substate5AndQueueFrontEndMode33ScenarioScena0300_801FC944(s32 arg0, s32 arg1);
s32 func_801FC9DC(s32 arg0, s32 arg1);
s32 func_801FCB4C(s32 arg0, s32 arg1);
s32 requestPrimaryState5Substate10AndQueueFrontEndMode1FScenarioScena0300_801FCAB8(s32 arg0, s32 arg1);

/* @source 0x801FC704
 * @behavior Route dispatcher of this overlay: it reads the shared unsigned
 * route word D_80143F00 and, for the six route values 0x25, 0x29, 0x32, 0x33,
 * 0x45 and 0x63, runs the matching overlay route handler — 0x25 runs
 * requestPrimaryState4Substate5AndQueueFrontEndMode33ScenarioScena0300_801FC944, 0x29 runs
 * func_801FC8AC, 0x32 runs
 * requestPrimaryState5Substate10AndQueueFrontEndMode1FScenarioScena0300_801FCAB8,
 * 0x33 runs
 * func_801FC7EC, 0x45 runs func_801FC9DC and 0x63 runs func_801FCB4C — and
 * returns that handler's result sign-extended from its low byte (sll/sra 24).
 * Every other route value returns 0. Both incoming packed record words are
 * forwarded unchanged: the body never writes $a0 or $a1, and all six callees
 * save them into $s0/$s1 and split them the same way (the first word's high
 * half and the second word compared against 0x40000/0x60000), which is the
 * pair the sibling dispatchers forward too. The handlers write the shared
 * state byte D_80146874 and sub-state byte D_80146875 before returning the
 * front-end mode id that queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0300_801FCC9C
 * consumes. The route word is compared as a signed int after its zero-extended
 * halfword load, so the dispatch tree is the compiler's balanced equality
 * chain over 0x25..0x63 rather than a table. The 0x18-byte frame only keeps
 * $ra across the selected call; no jal in the image targets this address, its
 * only image word is 0x801FC704 at 0x801FCFC8, word 17 of the overlay's
 * 32-word code-pointer run at 0x801FCF84.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 dispatchRouteHandlerScenarioScena0300_801FC704(s32 arg0, s32 arg1) {
  switch (D_80143F00) {
  case 0x25:
    return (s8)requestPrimaryState4Substate5AndQueueFrontEndMode33ScenarioScena0300_801FC944(arg0, arg1);

  case 0x29:
    return (s8)func_801FC8AC(arg0, arg1);

  case 0x32:
    return (s8)requestPrimaryState5Substate10AndQueueFrontEndMode1FScenarioScena0300_801FCAB8(arg0, arg1);

  case 0x33:
    return (s8)func_801FC7EC(arg0, arg1);

  case 0x45:
    return (s8)func_801FC9DC(arg0, arg1);

  case 0x63:
    return (s8)func_801FCB4C(arg0, arg1);
  }

  return 0;
}
