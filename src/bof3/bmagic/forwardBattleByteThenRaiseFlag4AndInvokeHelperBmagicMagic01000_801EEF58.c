#include "bof3/bof3.h"

void func_800A9820(u8 arg0);
void func_801E5988(void);

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * func_800A9820 consumes and that pickTargetStoreGlobal publishes. It lives
 * outside this overlay, so it keeps its address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind the
 * same word (BATTLE_GLOBAL_HALF_62E8); it lives outside this overlay, so it keeps
 * its address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEF58
 * @behavior Reads the shared battle byte at 0x80146384 with an unsigned byte
 * load, passes the zero-extended value to the shared helper func_800A9820,
 * raises bit 0x4 of the shared battle global halfword at 0x801462E8 (unsigned
 * halfword load, ori, halfword store) and then invokes the shared helper
 * func_801E5988. It takes no arguments, returns nothing and writes no other
 * state; the 0x18-byte frame only keeps $ra across the two calls. Both shared
 * words are reached through plain global accesses, which is what the original
 * shows: the byte load is materialised before the frame is created because the
 * call argument is its only consumer, while the halfword store re-materialises
 * its address into $at instead of keeping one base register live across the
 * call the way the sibling raiseFlag4AndInvokeHelper entries do. The 72 bytes
 * are byte-identical to seven other unlifted overlay copies (emi/bmagic/
 * magic008/00@0x801EF010, magic015/00@0x801EF200, magic020/03@0x801EF350,
 * magic117/00@0x801EEFB8, magic145/03@0x801EEDE4, magic146/03@0x801EF004 and
 * emi/boss/boss050/16@0x800C1AEC); none of those copies is lifted, so this
 * source was seeded from the original bytes instead of cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteThenRaiseFlag4AndInvokeHelperBmagicMagic01000_801EEF58(void) {
  func_800A9820(D_80146384);
  D_801462E8 |= 4;
  func_801E5988();
}
