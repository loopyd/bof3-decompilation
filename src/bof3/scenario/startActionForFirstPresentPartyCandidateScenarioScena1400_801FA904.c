#include "bof3/bof3.h"

/* @source 0x80146254 @kind unknown — shared live count of the 0x140-byte-stride
 * local work records. */
extern u8 D_80146254;
/* @source 0x80145F09 @kind unknown — party byte (+0x79) of the 0x140-byte-stride
 * local work records based at 0x80145E90. */
extern u8 D_80145F09[];
/* @source 0x80143BB0 @kind unknown — shared front-end phase byte. */
extern u8 D_80143BB0;

void func_80150224(s32 arg0);

/* @source 0x801FA904
 * @behavior Overlay party-scan action starter: it walks the four candidate party
 *           ids of the caller's byte row in order and scans the D_80146254 live
 *           0x140-byte-stride local work records for one whose party byte (+0x79,
 *           at D_80145F09) equals the candidate. On the first match it starts
 *           that candidate's front-end action - the paired halfword row entry
 *           through func_80150224 - publishes phase 2 in the shared front-end
 *           phase byte D_80143BB0 and returns the action id, and when no live
 *           record carries any of the four candidates it returns 0xFFFF. The
 *           rows it receives are both four entries wide: the caller passes
 *           0x801FC450 ({8, 2, 5, 4}) with 0x801FC458 ({2, 3, 4, 5}) at
 *           0x801F9938 and 0x801FC454 ({4, 5, 2, 8}) with 0x801FC460
 *           ({6, 7, 8, 9}) at 0x801F9A30, the two in-image jal sites inside
 *           func_801F9834. The candidate byte is the same +0x79 work-record byte
 *           the exact sibling findWorkByte79CandidateIndexWorld02Area10313_801F56B0
 *           scans, and that sibling's candidate-table/record-loop form is the
 *           seed shape; it is shape evidence only, because a payload scan of
 *           every out/binaries file finds this 196-byte span in this target
 *           alone. No image word holds the address itself; it is reached only
 *           through the two jal sites. The 0x18-byte frame keeps $ra and $s0:
 *           $s0 holds the action-row pointer across the func_80150224 call for
 *           the returned reload, the candidate-row pointer stays in $t0 and dies
 *           before that call, and the record count is re-read for the inner
 *           loop's bottom test because the body calls out. The inner loop keeps
 *           the record offset in its own local so the D_80145F09 access stays
 *           the folded lui/addu/lbu form rather than a hoisted table base.
 *           The returned element is read a second time through a pointer local
 *           rather than as `actions[i]` again: the argument read is emitted
 *           before the call, and the returned read must sit after the phase
 *           store, which only happens when the second read is a scalar
 *           dereference. Written as a subscript it is an in-struct reference and
 *           the pre-reload scheduler lifts it above the store, which moves the
 *           phase constant into a different register (measured 46/49 with the
 *           load and the store exchanged at 0x801FA970).
 * @status exact
 * @match 100.00
 * @residual none
 */
u16 startActionForFirstPresentPartyCandidateScenarioScena1400_801FA904(
    u8* candidates, s16* actions) {
  u8 i;
  u8 j;
  u32 offset;
  s16* action;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < D_80146254; j++) {
      offset = j * 0x140;
      if (D_80145F09[offset] == candidates[i]) {
        func_80150224(actions[i]);
        D_80143BB0 = 2;
        action = &actions[i];
        return *action;
      }
    }
  }
  return 0xFFFF;
}
