#include "bof3/bof3.h"

extern int rand(void);

/* Scratchpad pointer cell holding this overlay's work record. The cell is
 * reloaded for every access, which is what the original bytes show. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Front-end phase byte of the shared main executable: the front-end handlers
 * that own it treat 2 as busy and every other value as free. While it reads
 * non-zero this overlay only rebuilds the record projection. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind unknown */

/* Record builders of this overlay: the routine at 0x801F4798 takes the record
 * addresses +0x34 and +0x18, the routine at 0x801F4614 runs the record table
 * update and 0x801F45C8 builds the step tables. The builder at 0x801F4798 and
 * the update at 0x801F4614 carry their own target-map rows; 0x801F45C8 still
 * sits on an asm subsegment. */
void drawLineGouraudFanAndTwoGouraudQuadsBetweenRecordPoints34And18World03Area14813_801F4798(
    s32 arg0, s32 arg1);
void updateRecordTableFromProbeOffsetsAndStepFoundRecordOctantWorld03Area14813_801F4614(
    void);
void func_801F45C8(void);

/* @source 0x801F435C
 * @behavior Handler at word 1 of the state-byte-1 dispatch run based at
 *           0x801F6160, reached from the entry handler at 0x801F42C0 that
 *           writes 1 into the record byte at +1. While the front-end phase
 *           byte D_80143BB0 reads non-zero the body only re-runs the record
 *           builder at 0x801F4798 on the record addresses +0x34 and +0x18 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044. Otherwise it integrates that record first: the delta
 *           word at +0x0C is added to the position word at +0x18, the
 *           position word at +0x1C receives the delta word at +0x10, and the
 *           countdown byte at +9 is decremented. It then re-runs the record
 *           builder and the record table update at 0x801F4614. While the
 *           countdown byte is still non-zero the handler stops there. Once it
 *           reaches zero the handler advances the route byte at +8 by 2 modulo
 *           8, builds the step tables through 0x801F45C8, restarts the
 *           countdown with 0x50 and rolls rand() % 45 into the pacing byte at
 *           +0xA. A non-zero pacing byte costs one countdown step and moves
 *           the record to state 2 of the same dispatch run through the record
 *           byte at +1; a zero pacing byte keeps the restarted countdown and
 *           leaves the record in state 1. Takes no arguments, returns nothing
 *           and keeps a 0x18-byte frame whose only saved register is ra at
 *           +0x10; the front-end gate runs before the frame is created and the
 *           scratchpad record pointer cell is reloaded for every store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkPositionThenRestartCountdownWithRandomPacingAndSetState2WhenPacingNonZeroWorld03Area14813_801F435C(
    void) {
  u8* work;
  u8* next;
  u8* rec;

  if (D_80143BB0 != 0) {
    drawLineGouraudFanAndTwoGouraudQuadsBetweenRecordPoints34And18World03Area14813_801F4798(
        (s32)(D_1F800044 + 0x34), (s32)(D_1F800044 + 0x18));
    return;
  }
  work = D_1F800044;
  FIELD_REF(s32, work, 0x18) += FIELD_REF(s32, work, 0x0C);
  work[9]--;
  next = D_1F800044;
  FIELD_REF(s32, work, 0x1C) += FIELD_REF(s32, work, 0x10);
  drawLineGouraudFanAndTwoGouraudQuadsBetweenRecordPoints34And18World03Area14813_801F4798(
      (s32)(next + 0x34), (s32)(next + 0x18));
  updateRecordTableFromProbeOffsetsAndStepFoundRecordOctantWorld03Area14813_801F4614();
  rec = D_1F800044;
  if (rec[9] != 0) {
    return;
  }
  rec[8] = (rec[8] + 2) & 7;
  func_801F45C8();
  D_1F800044[9] = 0x50;
  D_1F800044[0xA] = rand() % 45;
  if (D_1F800044[0xA] != 0) {
    D_1F800044[9]--;
    D_1F800044[1] = 2;
  }
}
