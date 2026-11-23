#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named symbol so each
 * access is one lui plus offset load on the shared cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Sub-state handler table the tail dispatches through. */
extern void (*D_801F544C[])(void); /* @source 0x801F544C @kind table */

/* The payload data blob at 0x801F5434 opens with the two handler pointers of
 * the work-byte-1 dispatch table (0x801F2CD4 and this handler, at 0x801F2D44)
 * and is followed by the four four-byte coordinate rows this handler walks.
 * The rows are reached through that leading pair, which is why each row read
 * carries its displacement in the load's own halfword. */
struct World03Area14514StepHandlersAndRowTable {
  void (*step_handlers[2])(void);
  u8 rows[4][4];
};

extern struct World03Area14514StepHandlersAndRowTable
    D_801F5434; /* @source 0x801F5434 @kind data */

/* Drawing helper that projects the record's two point slots at that record's
 * fixed depth words and draws between them; still an asm row in this overlay's
 * Splat listing, so its parameter roles stay structural. */
extern void func_801F2F30(void *point_a, void *point_b); /* @source 0x801F2F30 */

/* @source 0x801F2D44
 * @behavior Overlay state handler 1 of the local work-byte-1 dispatch table
 *           based at 0x801F5434. It walks the four four-byte rows that follow
 *           that table's two entry words, each row holding the two coordinate
 *           pairs (x0, y0, x1, y1) of one stage, and for each row stores both
 *           pairs into the work record published at the scratchpad pointer cell
 *           0x1F800044: words +0x0C and +0x10 take the row's first pair and
 *           words +0x18 and +0x1C its second, each coordinate widened to 16.16
 *           and offset by half a unit through the low bit 0x8000, the same
 *           centre-of-unit form the shared position pairs use. It then calls
 *           the payload drawing helper at 0x801F2F30 with the addresses of
 *           those two pairs, which projects both and draws between them, so the
 *           loop draws one stage per row and leaves the record holding the last
 *           row's pair. After the fourth row it re-reads the pointer cell,
 *           takes the record's sub-state byte at +0x02 and calls that entry of
 *           the local handler table based at 0x801F544C. The loop counter is an
 *           unsigned byte, which is why it is masked by 0xFF both when scaled
 *           by four for the row index and when compared with the row count, and
 *           the pointer cell is re-read for the dispatch because the drawing
 *           call sits between the two reads.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderPointPairsFour(
    void) {
  u8 *work;
  u8 i;

  for (i = 0; i < 4; i++) {
    work = D_1F800044;
    *(s32 *)(work + 0x0C) = (D_801F5434.rows[i][0] << 16) | 0x8000;
    *(s32 *)(work + 0x10) = (D_801F5434.rows[i][1] << 16) | 0x8000;
    *(s32 *)(work + 0x18) = (D_801F5434.rows[i][2] << 16) | 0x8000;
    *(s32 *)(work + 0x1C) = (D_801F5434.rows[i][3] << 16) | 0x8000;
    func_801F2F30(work + 0x0C, work + 0x18);
  }
  D_801F544C[D_1F800044[2]]();
}
