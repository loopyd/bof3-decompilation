#include "bof3/bof3.h"

/* Shared byte the mode arms of this family set and test: the dispatcher
 * 0x801F3F34 stores 2 here and tests it back, and other overlays reuse the same
 * cell as their own state byte. This step hands its projected pair to the
 * cell-cue helper only while the byte reads 5, and the role of that value is
 * not established, so the byte stays address-anchored. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Shared progress byte of the front-end mode machine. The dispatcher
 * 0x801F3F34 seeds it with 1 and the mode arms step it to 10 and 20, which is
 * why the three callers of this step test it against those values only after
 * this step has answered nonzero. */
extern u8 D_80146867; /* @source 0x80146867 @kind data */

/* Pointer cell of the shared record whose halfword at +0x0A the idle path steps
 * back by 2; the same cell is the armed-record slot of the world step family. */
extern u8 *D_80148210; /* @source 0x80148210 @kind data */

/* Overlay helper that cues the projected cell column pair: it converts each
 * word to a cell coordinate with a 16-bit arithmetic shift and queues the work
 * byte selected from words +0x18 and +0x1C through the shared cue queue
 * 0x801A4BC0, for the cell and the cell one row up. */
/* @source 0x801F3C4C */
extern void func_801F3C4C(s32 x, s32 y);

/* @source 0x801F3980
 * @behavior Projects the current scratchpad work object's position pair into
 *           the caller's two words. It loads the work object from the
 *           scratchpad pointer cell 0x1F800044 once and stores word +0x34
 *           minus 0x8000 through the first pointer and word +0x38 minus 0x8000
 *           through the second, materialising the -0x8000 immediate once and
 *           adding it to both words. While the shared byte 0x80143BB0 reads 5
 *           it hands the pair to the overlay cell-cue helper 0x801F3C4C, with
 *           the first word reloaded from the caller's cell and the second
 *           taken from the register that stored it. When the shared progress
 *           byte 0x80146867 reads zero it steps the shared record's halfword
 *           at +0x0A back by 2, on the record the pointer cell 0x80148210
 *           publishes read freshly after the gate, and answers 0; otherwise it
 *           answers 1, the constant the original materialises in that gate's
 *           branch delay slot. The 0x18-byte frame saves only $ra.
 *
 * Name evidence: this target's three mode arms 0x801F37A8, 0x801F3894 and
 * 0x801F43B0 declare the two pointers as the work position pair and only act on
 * this step's byte answer, then read 0x80146867 themselves; the dispatcher
 * 0x801F3F34 documents 0x80146867 as the front-end progress byte and
 * 0x80143BB0 as the byte its mode 10 arm arms with 2 and its mode 11 arm tests;
 * and 0x801F3C4C turns the returned pair into cell coordinates and queues cue
 * codes for them. The 0x8000 bias on work words +0x34/+0x38 is the same fixed
 * point offset the world family adds and removes elsewhere
 * (seedFrontEndRecordHandler37FromWorkPositionWorld01Area04313_801F2E28,
 * advanceWorkWord34By8000WhenWorkByte79Is2World01Area04013_801F2C74), so it
 * stays a literal. No jal and no pointer word in the payload reaches this body
 * from outside the three mode arms, so nothing about the argument roles is
 * hidden here.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 projectCueCellsProgress(
    s32 *x, s32 *y) {
  s32 *work;

  work = SPAD_PTR_SLOT(s32, 0x44u);
  *x = work[0xD] - 0x8000;
  *y = work[0xE] - 0x8000;
  if (D_80143BB0 == 5) {
    func_801F3C4C(*x, *y);
  }
  if (D_80146867 == 0) {
    FIELD_REF(u16, D_80148210, 0x0A) -= 2;
    return 0;
  }
  return 1;
}
