#include "bof3/bof3.h"

/* Shared main-RAM scenario progress byte at 0x80146864. It is a plain byte
 * load here and this target's generated undefined_syms_auto row already binds
 * the address, so the source keeps the plain map name. */
extern u8 g_ScenarioProgress;

void func_80196070(void);
void func_801F7484(void);
void func_801F7568(void);

/* @source 0x801F7444
 * @behavior Work-object step handler of this overlay, reached while the
 * dispatch byte 0x01 of the work object published at scratchpad cell 0x1F800044
 * reads 4. It is word 4 of the sub-run D_801FBEFC (0x801FBF0C) that the frame
 * dispatcher func_801F7198 indexes with that byte, and word 7 of the
 * twelve-word handler run D_801FBEF0. It takes no arguments and returns
 * nothing. It reads the shared scenario progress byte g_ScenarioProgress
 * (0x80146864) with `lbu` (`lui $v1,0x8014` / `lbu $v1,0x6864($v1)`) and
 * compares it against 0x35 (`addiu $v0,$zero,0x35` / `bne $v1,$v0`): only when
 * that byte equals 0x35 does it call the shared work-area reset func_80196070,
 * whose exact scena0300 lift at 0x801F7334 records it clearing the work-object
 * header bytes 0x00-0x04. Both paths then call the overlay helpers func_801F7484
 * and func_801F7568 in that order with no arguments and with any result
 * discarded, so the pair runs on every call. Every jal delay slot stays a nop,
 * and the 0x18-byte frame keeps only $ra at 0x10, stored in the delay slot of
 * that compare. It reads and writes no other state. No word in this payload
 * encodes a jal to 0x801F7444, so the two handler runs are its only references.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetWorkAreaAtProgress35ThenInvokeTwoHelpersScenarioScena1300_801F7444(void) {
  if (g_ScenarioProgress == 0x35) {
    func_80196070();
  }
  func_801F7484();
  func_801F7568();
}
