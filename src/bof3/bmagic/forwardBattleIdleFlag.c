#include "bof3/bof3.h"

/* Battle-byte forwarding helper of the shared main executable at 0x800A9820.
 * No map row of this target or of the shared PsyQ maps binds that address, so
 * the callee is bound by this tagged prototype, the spelling the exact sibling
 * lifts of the neighbouring magic overlays use for the same call. */
extern void func_800A9820(u8 arg0); /* @source 0x800A9820 @kind function */

/* Shared no-argument helper of the main executable at 0x801E5988. This target's
 * generated undefined_funcs_auto.txt bound the address while this span was still
 * an asm row, and the sibling lifts of this target name the same address through
 * the same prototype. */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* Unsigned battle byte in main RAM, outside this overlay's image. This target's
 * generated undefined_syms_auto.txt binds the address, so no target-map row is
 * needed. The original loads it at byte width (lbu), which is why the declared
 * width is a byte. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle flag halfword in main RAM, outside this overlay's image. The
 * original reads and writes it at halfword width (lhu / ori 0x4 / sh), so the
 * declared width is an unsigned halfword and the read-modify-write keeps the
 * volatile spelling the exact sibling lift of this body measures. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EF060
 * @behavior Entry 3 (the last) of the four-entry work byte 1 handler run at
 *           0x801EEC04 (0x801EEC90, 0x801EEE94, 0x801EEFE0, 0x801EF060), a run
 *           that occupies the overlay's own table words and that the exact
 *           sibling dispatcher dispatchWorkByte1HandlerBmagicMagic15403
 *           _801EEC24 copies into its frame and invokes with no arguments, so
 *           the payload holds no jal to this address. It reads the scratchpad
 *           work-object pointer cell at 0x1F800044 once before its frame exists
 *           and tests the work object's unsigned byte 0xB (lbu / bnez). While
 *           that byte is nonzero the handler returns without writing anything.
 *           At zero it forwards the unsigned battle byte at 0x80146384 to the
 *           shared main-executable helper func_800A9820 (lbu $a0 / jal), raises
 *           mask 0x4 in the shared battle halfword at 0x801462E8 (lhu / ori 0x4
 *           / sh) and then calls the shared no-argument helper func_801E5988.
 *           The 0x18-byte frame keeps only $ra at 0x10; the handler takes no
 *           arguments, returns nothing and reads or writes no other state. The
 *           roles of battle byte 0x80146384 and of the flag bit 0x4 are not
 *           established in this payload, so the name describes the writes. The
 *           96-byte body is byte-identical, by payload span comparison, to the
 *           body at 0x801EF050 of emi/bmagic/magic124/03.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleIdleFlag(
    void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] == 0) {
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
