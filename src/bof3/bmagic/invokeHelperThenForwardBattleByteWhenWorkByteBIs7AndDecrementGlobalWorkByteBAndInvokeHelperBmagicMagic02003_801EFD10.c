#include "bof3/bof3.h"

void func_800A9820(u8 arg0);
void func_801E5988(void);

/* Shared no-argument update helper of the main executable, outside this
 * overlay's image. This target's generated undefined-funcs map bound it at
 * 0x8014D978 while this span was still assembly, and the exact sibling
 * stepWorkByteBAndClearWorkByte2BWithRecordState12BWhenStepReportsDone
 * PlcharPlp56700_801D0208 calls the same helper and discards its answer. Here
 * the answer gates the whole body: the original issues the jal with a nop
 * delay slot and then truncates $v0 to eight bits (andi $v0,$v0,0xFF) before a
 * beqz, so the declared return is an unsigned byte. */
extern u8 func_8014D978(void);

/* Scratchpad work-object pointer cell, spelled through this address-derived
 * symbol, the same named cell the exact lifts of this target already use. A
 * named cell symbol lets the assembler fold the constant address into the load
 * (lui $v0,0x1F80 / lw $v0,0x44($v0)). */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared battle byte at 0x80146384 of the main executable: the zero-extended
 * byte the helper func_800A9820 consumes, the same argument the exact sibling
 * raiseFlag4ThenForwardBattleByteAndInvokeHelperBmagicMagic05703_801EFEEC
 * forwards. Its role is not established here, so it keeps its address-derived
 * name; this target's generated undefined-syms map already binds the address. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * The body reads it once for the byte-0xB decrement, the same cell and the same
 * read-modify-write the exact sibling
 * decrementGlobalWorkByteBThenClearWorkBytesBmagicMagic12303_801EF550 uses, so it
 * keeps this target's address-derived name; the generated undefined-syms map
 * already binds the address. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EFD10
 * @behavior Step handler of the battle work object. This 0x78-byte span is word
 *           at 0x801EFD10 in the 0x10-byte code-pointer pair that
 *           func_801EFAF4 stores into its own frame at 0x10($sp) and 0x14($sp)
 *           and indexes with the work object's unsigned byte 2 (lbu 2 / sll 2 /
 *           addu / lw / jalr), so this body runs with no arguments and returns
 *           no value. It first invokes the shared no-argument update helper
 *           func_8014D978, truncates its answer to eight bits (andi $v0,$v0,0xFF)
 *           and returns at once through its jr $ra when that byte is zero. Only
 *           on the non-zero arm does it materialize the work-object pointer cell
 *           at 0x1F800044 (lui $v0,0x1F80 / lw $v0,0x44($v0)) and read the
 *           object's unsigned byte 0x0B; when that byte is exactly 7 it loads
 *           the shared battle byte at 0x80146384 (lui $a0,%hi / lbu %lo) and
 *           passes it to the helper func_800A9820, which returns no value that
 *           the body reads. Every non-zero answer then reaches the same tail:
 *           the body loads the shared battle work record pointer at 0x801EB4E0
 *           once, decrements that record's byte 0x0B at byte width (lbu 0xB /
 *           addiu -1 / sb 0xB, the stored value is never tested) and invokes the
 *           shared helper func_801E5988, the store filling the call delay slot.
 *           The 0x18-byte frame keeps only $ra at 0x10 across the calls, and
 *           every branch and call delay slot in the span is nop. The roles of
 *           work byte 0x0B, of that byte on the 0x801EB4E0 record and of the
 *           shared battle byte at 0x80146384 are not established by this span,
 *           so they stay address-anchored. A byte search over every payload
 *           under out/binaries found exactly one copy of this 0x78-byte span,
 *           and a repetition with the three jal immediate words masked found
 *           none either, so the clone lever was declined. The span opens with
 *           addiu $sp,-0x18 and closes in jr $ra through its own frame, so the
 *           data lever did not apply, and it sits between the reviewed
 *           boundaries at 0x801EFBA0, whose span ends before this address, and
 *           0x801EFD88, so the carve lever did not apply either.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperThenForwardBattleByteWhenWorkByteBIs7AndDecrementGlobalWorkByteBAndInvokeHelperBmagicMagic02003_801EFD10(
    void) {
  u8 *work;

  if (func_8014D978()) {
    work = D_1F800044;
    if (work[0xB] == 7) {
      func_800A9820(D_80146384);
    }
    D_801EB4E0[0xB]--;
    func_801E5988();
  }
}
