#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u16 D_801F37FC[]; /* @source 0x801F37FC @kind table */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* @source 0x801F2C44
 * @behavior Reads the shared signed gate byte D_801490C7, indexes the shared
 *           halfword table D_801F37FC with it (`sll $v0,$v1,1`, `addu
 *           $at,$at,$v0`, `lhu`) and stores the selected halfword into the
 *           shared status halfword D_801490A8. It then selects on the same gate
 *           byte and stores 3 into the shared scenario progress byte
 *           g_ScenarioProgress when the gate reads 0 and 4 when it reads 1,
 *           leaving the progress byte untouched for any other value. Takes no
 *           arguments, calls nothing and returns nothing: a frameless leaf
 *           whose 0x50-byte span carries no prologue and closes on `jr $ra` and
 *           its delay `nop`.
 *
 *           Byte evidence from this span: `lui`/`lb` reads 0x801490C7 signed,
 *           the scaled index fills the one `lhu` off 0x801F37FC, the single `sh`
 *           to 0x801490A8 carries that loaded halfword, the two arm constants
 *           are materialised in $v0 and fill the branch delay slots of the
 *           equality tests, both arms merge on one `sb` to 0x80146864, and any
 *           other value falls straight to the return through the `j` at
 *           0x801F2C78.
 *
 *           Seed: the same-target sibling
 *           stateStatusOr2Gate
 *           at 0x801F2C04 in this payload binds the same three shared cells, and
 *           its lift measured byte-identical, so its statement order, `switch`
 *           topology and declaration style are transcribed here with this
 *           span's own table read added in front of the status store and this
 *           span's own arm constants written from the words above. A scan of
 *           every payload under out/binaries found no byte-identical copy of
 *           this 0x50-byte range, so no clone was claimed; the four-word-apart
 *           spans elsewhere in world00/world01/world02/exe keep their own table
 *           address, `j` target and arm constants and served as shape evidence
 *           only.
 *
 * Naming evidence: the status halfword is a real table read rather than a pair
 * of immediates, which is the `...FromGateTable...` grammar already used by
 * setStatus54OrFFFFFromGateTableThenWaitFlag6Cue106ClaimAndSetFlagWorld01Area04113_801F3B48
 * for the same gate byte into the same status halfword, and the two case
 * constants and their `switch` shape are those of the sibling above; the table
 * words at 0x801F37FC are zero on disc, so the name names the status source
 * instead of values it cannot see.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatusGateGate(
    void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = D_801F37FC[gate];
  switch (gate) {
  case 0:
    g_ScenarioProgress = 3;
    break;
  case 1:
    g_ScenarioProgress = 4;
    break;
  default:
    break;
  }
}
