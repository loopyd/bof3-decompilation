#include "bof3/bof3.h"

/* Work record the main program publishes through the scratchpad pointer cell
 * 0x1F800044. The original re-reads the cell before each access group below,
 * because a store through the loaded pointer may alias the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared main-program frame counter, bound by the shared map: bit 0 is the
 * frame parity and the low three bits gate the slower per-frame updates. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* This payload's rotated-quad draw pass, carried by the overlay as a 16-bit
 * angle: it samples the texture page at (0x340, 0x100) through GetTPage, takes
 * the drawn quad's size from record words +0x0C/+0x10, its position from
 * +0x18/+0x1C and the rotation angle from the halfword it is handed, which it
 * masks to 16 bits and scales by 16, and gives the quad the 0xBF-by-0x68
 * texture UV rectangle. */
void renderSpinQuad(u16 angle); /* @source 0x800C23D4 */

/* @source 0x800C1B40
 * @behavior Overlay state 3 handler, entry 3 of the handler run based at
 * 0x800C2714 that the byte-3 dispatcher callStateTable2714Boss
 * indexes with work-record byte 3, so no jal names this span. It loads the work
 * record published at the scratchpad pointer cell 0x1F800044 and adds that
 * record's byte 0x0A, the per-frame spin step the preceding step seeded with 1,
 * into its accumulated spin word 0x20. While the record word 0x0C is at least
 * 0x29 it decrements that word, so the word settles at 0x28, and on bit 0 of the
 * shared frame counter D_80143E6C it also increments word 0x1C, the second of
 * the two position words the draw pass adds to each quad corner. It then
 * re-reads the cell and decrements word 0x10 under the same 0x29 bound with no
 * frame-parity gate. It re-reads the cell a third time for the drawn angle,
 * takes the accumulated spin word as a 16-bit value and runs this overlay's
 * rotated-quad draw pass at 0x800C23D4. While any of the low three bits of
 * D_80143E6C is set it re-reads the cell and increments the spin step byte
 * 0x0A, so the spin accelerates on seven frames of eight, and it then re-reads
 * the cell once more to test that byte against 0x40: at 0x40 and above it
 * increments the record's state byte 0x03, which moves the byte-3 dispatch on to
 * 0x800C1C44. The payload holds no jal naming 0x800C1B40; the one pointer word
 * that names it is 0x800C2720, entry 3 of the run based at 0x800C2714.
 *
 * Representation notes: two spellings carry the match and both were measured.
 * Taking D_80143E6C into `frame` before the decrement store puts the counter
 * load ahead of the decrement value and lets that store fill the branch delay
 * slot; reading the counter at the test instead measured 53 of 65 instructions
 * with two extra nops, the counter's load-delay slot and the branch slot empty.
 * The two trailing accesses re-read the cell into one shared local, which keeps
 * both in $v1; giving the second one its own local measured 65 of 65 as well,
 * and reusing the earlier `work` local for the trailing blocks measured 49 of
 * 67, with reusing it for only the last block leaving both in $a0 at 60 of 65
 * and a first difference at +0xC4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void accumulateSpinShrinkSpin(
    void) {
  u8 *work;
  u8 *record;
  u32 frame;

  work = D_1F800044;
  FIELD_REF(u32, work, 0x20u) += FIELD_REF(u8, work, 0x0Au);
  if (FIELD_REF(s32, work, 0x0Cu) >= 0x29) {
    frame = D_80143E6C;
    FIELD_REF(s32, work, 0x0Cu)--;
    if (frame & 1u) {
      FIELD_REF(s32, work, 0x1Cu)++;
    }
  }
  work = D_1F800044;
  if (FIELD_REF(s32, work, 0x10u) >= 0x29) {
    FIELD_REF(s32, work, 0x10u)--;
  }
  renderSpinQuad(FIELD_REF(u16, D_1F800044, 0x20u));
  if (D_80143E6C & 7u) {
    record = D_1F800044;
    FIELD_REF(u8, record, 0x0Au)++;
  }
  record = D_1F800044;
  if (FIELD_REF(u8, record, 0x0Au) >= 0x40u) {
    FIELD_REF(u8, record, 0x03u)++;
  }
}
