#include "bof3/bof3.h"

void func_800A9820(u8 arg0);
void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. It lives outside this overlay,
 * so it keeps its address-qualified name; this target's generated
 * undefined_syms_auto.txt already binds the address because the remaining
 * assembly spans in this payload load it, so no target-map row is added. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* Shared battle target byte at 0x80146384: the zero-extended battle byte that
 * func_800A9820 consumes. It lives outside this overlay, so it keeps its
 * address-qualified name, and this target's generated undefined_syms_auto.txt
 * already binds the address. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EEFEC
 * @behavior Raises bit 0x4 of the shared battle global halfword at 0x801462E8,
 *           then forwards the shared battle byte at 0x80146384 to the shared
 *           helper func_800A9820 and invokes the shared helper func_801E5988
 *           with no arguments. The original materialises the halfword address
 *           once in a register (lui / addiu into $v1), reads it unsigned (lhu),
 *           ORs the immediate 0x4 and stores it back through that same register,
 *           the store filling the delay slot of the first call; the byte
 *           argument is a separate unsigned byte load (lui $a0 / lbu) that the
 *           scheduler hoists above the ori. The 0x18-byte frame only keeps $ra
 *           at 0x10 across the two calls. It takes no arguments, returns
 *           nothing and writes no other state.
 *           Reachability: this 0x40-byte span is word 2 of the three-word code
 *           pointer table at 0x801EEC04 (0x801EECB0, 0x801EED2C, 0x801EEFEC).
 *           The dispatcher func_801EEC50 loads those three words, copies them
 *           into its 0x28-byte frame and indexes the copy with the scratchpad
 *           work object's unsigned byte 0x01 (lbu / sll 2 / addu / lw / jalr),
 *           so this body runs on the frames where work byte 0x01 is 2 and no jal
 *           word in the image names it. The 64 bytes are not byte-identical to
 *           any other span in the 396 payloads under out/binaries, so the body
 *           was seeded from the original bytes instead of cloned; statement
 *           order and the through-one-register read-modify-write follow the
 *           exact siblings raiseFlag4AndInvokeHelperBmagicMagic05603_801EF17C
 *           and forwardBattleByteThenRaiseFlag4AndInvokeHelperBmagicMagic01000
 *           _801EEF58 in this module.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4ThenForwardBattleByteAndInvokeHelperBmagicMagic05703_801EEFEC(void) {
  u16 *flags = (u16 *)&D_801462E8;

  *flags |= 4;
  func_800A9820(D_80146384);
  func_801E5988();
}
