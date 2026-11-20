#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_80146875;
extern u8 g_ScenarioProgress;

/* @source 0x801F389C
 * @behavior Reads the shared gate byte D_801490C7, stores 0xFFFF into the
 * shared status halfword D_801490A8, and then stores a progress value into the
 * shared scenario progress byte g_ScenarioProgress together with a substate
 * value into the shared byte D_80146875: 0x28 and 0x15 while the gate byte is
 * zero, 0x32 and 0x19 while it is nonzero. Takes no arguments, calls nothing
 * and returns nothing; the 0x4C-byte span is a frameless leaf that closes on
 * `jr $ra` and its delay nop.
 *
 * Byte evidence from this span: the signed `lb` of 0x801490C7 is hoisted above
 * the single `ori`/`sh` pair that fills 0x801490A8, the zero test on that gate
 * value is a `bnez` whose delay slot materialises the 0x28 progress literal,
 * and the two arms each hold one `lui`/`sb` pair for 0x80146864. The trailing
 * substate stores of both arms merge into one `lui`/`sb` pair for 0x80146875
 * after the join, so each arm puts its own 0x15 or 0x19 into $v0 and the arm
 * that falls through the `j` back to the join leaves that value in the `j`
 * delay slot.
 *
 * Naming evidence: the in-scope exact siblings
 * setStatusFFFFSubstate2IfGateElse3World04Area17513_801F386C and
 * setStatus5AAndPtrByte7AFFIfGateElseFFFFWorld04Area17513_801F38E8 record
 * 0x801490A8 as the shared status halfword, 0x801490C7 as the shared gate byte
 * and 0x80146875 as the shared substate byte, and the world family names
 * 0x80146864 the shared scenario progress byte
 * (setStatusFFFFAndScenarioProgress64Or65ByGateWorld00Area01013_801F2C04). The
 * roles of the two progress/substate pairs beyond this span are not
 * established, so the name states the stored constants and their gate order.
 *
 * Shape evidence: ten payloads of this family (emi/world04/area176 through
 * area185, slot 13, each at payload offset 0x48) carry this 76-byte body word
 * for word except the absolute `j` that joins the arms to the substate store,
 * which shifts by exactly the 0xC54 delta between the two spans; every arm
 * literal and both progress-store cell words are equal there. Those twins sit
 * inside a leading bare `bin` segment and hold no lifted source, so this seed
 * was written from this span's own bytes with the family statement order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFProgress32Substate19IfGateElseProgress28Substate15World04Area17513_801F389C(void) {
  D_801490A8 = 0xFFFF;
  if (D_801490C7 == 0) {
    g_ScenarioProgress = 0x28;
    D_80146875 = 0x15;
  } else {
    g_ScenarioProgress = 0x32;
    D_80146875 = 0x19;
  }
}
