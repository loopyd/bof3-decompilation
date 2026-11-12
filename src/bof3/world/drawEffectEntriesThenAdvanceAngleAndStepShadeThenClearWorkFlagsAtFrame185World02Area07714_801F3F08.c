#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Resident helpers this overlay calls, declared the way its sibling
 * drawFourStaggeredTranslucentArcFansAroundPointWorld02Area07714_801F4550.c
 * declares them: func_8014E5A0 links the current primitive cursor into the cell
 * list and advances it by the passed size, and func_801AFE18 fills the
 * 0x20-byte projection scratch handed as arg0. */
void func_8014E5A0(u32 ot_index, u32 primitive_size);
void func_801AFE18(void* arg0);

/* Effect entry draw pass at 0x801F47C8, still an asm boundary of this payload so
 * it keeps its address anchor. It stamps the primitive cursor, turns
 * semi-transparency on, projects the entry and builds the entry's rotation from
 * the unsigned spin angle halfword at +0x24 and the extents at +0x10/+0x12,
 * which is why the pass below advances that halfword between frames. */
void func_801F47C8(u8* entry);

/* Shared work-area reset at 0x80196070, lifted from the main executable as
 * clearWorkFlags: it clears work bytes 0x00-0x04, the dispatch byte +0x01
 * included, so calling it ends this overlay's state. Out of the payload, so it
 * keeps the generated undefined_funcs_auto.txt binding and no map row. */
extern void func_80196070(void); /* @source 0x80196070 @kind function */

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;

/* @source 0x801F3F08
 * @behavior Overlay work-byte-1 handler, entry 1 of the local handler table at
 *           0x801F6268, which the dispatcher 0x801F3E04 indexes with the work
 *           byte +0x01. It arms the shared draw mode on the primitive cursor
 *           g_PrimCursor for the texture page GetTPage(0, 1, 0x380, 0x100) with
 *           both dither fields clear, appends the 0xC-byte draw-mode packet
 *           through func_8014E5A0 under ordering-table slot 1, pushes the
 *           shared matrix and fills the 0x20-byte projection scratch at +0x18
 *           of its frame through func_801AFE18. It then walks the 32 entries of
 *           the 0x2C-byte-stride effect table at 0x800E5504, the table the
 *           neighbouring handler entry 0 (0x801F3E48) seeds through
 *           func_801F4A68: each entry is drawn through func_801F47C8, its
 *           unsigned spin angle halfword at +0x24 is advanced by 0x10, and the
 *           unsigned halfword at +0x2A is raised by 0x20 while the work byte
 *           +0x09 is below 4 and lowered by 2 otherwise. After PopMatrix it
 *           reloads the work pointer from scratchpad slot 0x1F800044,
 *           increments the frame counter word at +0x0C and calls the shared
 *           work-area reset func_80196070 once that counter reaches 0xB9 (185),
 *           which clears the dispatch byte and ends the state. Takes no
 *           arguments and returns nothing; field roles stay offset-anchored
 *           beyond what the neighbouring handlers and the drawn callee read.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawEffectEntriesThenAdvanceAngleAndStepShadeThenClearWorkFlagsAtFrame185World02Area07714_801F3F08(
    void) {
  u8 scratch[0x20];
  u8* entry;
  u8 index;
  u8* work;
  s32 frame;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 1, 0x380, 0x100), 0);
  func_8014E5A0(1, 0x0c);
  PushMatrix();
  func_801AFE18(scratch);

  entry = PSX_PTR(u8, 0x800E5504u);
  index = 0;

  do {
    func_801F47C8(entry);
    work = workRecordCursor1F800044;
    FIELD_REF(u16, entry, 0x24u) = (u16)(FIELD_REF(u16, entry, 0x24u) + 0x10);
    if (work[9] < 4) {
      FIELD_REF(u16, entry, 0x2Au) = (u16)(FIELD_REF(u16, entry, 0x2Au) + 0x20);
    } else {
      FIELD_REF(u16, entry, 0x2Au) = (u16)(FIELD_REF(u16, entry, 0x2Au) - 2);
    }
    entry += 0x2c;
    index += 1;
  } while (index < 0x20);

  PopMatrix();

  work = workRecordCursor1F800044;
  frame = *(s32*)(work + 0x0c) + 1;
  *(s32*)(work + 0x0c) = frame;
  if (frame >= 0xB9) {
    func_80196070();
  }
}
