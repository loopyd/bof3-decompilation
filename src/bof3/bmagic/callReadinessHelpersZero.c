#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's handlers. The
 * original re-reads the cell after the helper calls (lui $v1,0x1F80 /
 * lw $v1,68($v1)) because a store through the pointer may alias the cell
 * object, so the pointer stays volatile and every access reloads it, which also
 * lets the assembler fold the constant address into the load. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle byte in main RAM, outside this overlay's image: the byte the
 * zero-countdown arm forwards to the helper at 0x800A9820. Splat binds the
 * address through this target's generated undefined_syms_auto.txt, so no
 * target-map row is added here. */
extern u8 D_80146384; /* @source 0x80146384 @kind data */

/* Shared readiness helper of the main executable at 0x8014DAEC, outside this
 * overlay's image. The exact sibling lift
 * invokeHelperAt100AndAdvanceWorkByte2WhenWorkByte9Is10ElseAdd4ToWorkByte9Then
 * DrawAndRunHelperBmagicMagic16103_801EF0E8 declares the same prototype; this
 * call site issues the jal and never tests $v0, so the answer is discarded. */
u8 func_8014DAEC(void);

/* Shared helper of the main executable at 0x800AFC98, outside this overlay's
 * image: invoked here with the constant pair (2, 4) and its result discarded,
 * so the original constrains neither argument width and word-width arguments
 * keep the default promotion. */
void func_800AFC98(s32 arg0, s32 arg1);

/* Shared helper of the main executable at 0x800A9820, outside this overlay's
 * image: this body forwards the shared battle byte 0x80146384 to it. */
void func_800A9820(u8 arg0);

/* @source 0x801EF06C
 * @behavior Entry 1 (index 1) of the five-entry handler run at 0x801EEC10
 *           whose words are 0x801EF024, 0x801EF06C, 0x801EF0E8, 0x801EF158 and
 *           0x801EF1BC. The work byte 2 dispatcher at 0x801EEF80 copies the run
 *           into its own 0x30-byte frame and invokes the entry that scratchpad
 *           work-object byte 2 selects, with no bounds test and with no jal
 *           anywhere in the payload naming this body. Per frame it calls the
 *           shared readiness helper at 0x8014DAEC and discards the returned
 *           byte, then re-materializes the work-object pointer cell at
 *           0x1F800044 and decrements that object's unsigned byte 9 in place
 *           (lbu / addiu -1 / sb), testing the stored byte truncated to eight
 *           bits (andi 0xFF / bnez), so the rest of the body runs only when that
 *           countdown reaches zero. On zero it invokes the shared helper at
 *           0x800AFC98 with the constant pair (2, 4), the 2 filling that test's
 *           branch delay slot, then forwards the shared battle byte 0x80146384
 *           to the helper at 0x800A9820, and finally re-reads the pointer cell
 *           and increments work byte 2 with a byte-width add, moving dispatch to
 *           run entry 2 at 0x801EF0E8. It takes no arguments and returns
 *           nothing; the 0x18-byte frame only keeps the return address at 0x10
 *           across the calls, and it reads or writes no other state. One scan of
 *           the extracted payloads found no other copy of the 0x7C-byte span in
 *           any target, and the nearest twin, the span at 0x801F1384 of
 *           emi/bmagic/magic008/00, differs from this body in exactly one word:
 *           the passed constant is 4 here against 0 there (payload byte
 *           comparison), so that twin is shape evidence and this body writes its
 *           own immediate. Statement order, cell re-materialization grouping and
 *           declaration style follow that twin, whose lift measured exact.
 *           Roles of work byte 9, of the three shared helpers and of the (2, 4)
 *           immediate are not established here, so they stay address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callReadinessHelpersZero(void) {
  func_8014DAEC();
  if (--g_battle_work[9] == 0) {
    func_800AFC98(2, 4);
    func_800A9820(D_80146384);
    g_battle_work[2]++;
  }
}
