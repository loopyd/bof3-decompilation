#include "bof3/bof3.h"

extern int rand(void);
void func_80196070(void);

/* Third shared work base of this overlay: the 0x140-byte-stride group the
 * overlay's third-base seeders publish as the scratchpad work cursor. */
extern u8 D_80146110[]; /* @source 0x80146110 @kind data */
/* Word at +0x34 of that base, the limit this handler tests. */
extern s32 D_80146144; /* @source 0x80146144 @kind unknown */
/* Published work pointer beside the scratchpad work cursor. */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */
/* Scratchpad work-pointer cell; the original re-reads it for the work byte +9
 * store and again for the mode-3 store after restoring it, so the cell keeps
 * its volatile form. */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80146865; /* @source 0x80146865 @kind unknown */
extern u8 D_80146866; /* @source 0x80146866 @kind unknown */
extern u8 D_80146867; /* @source 0x80146867 @kind unknown */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */
/* Shared pad word, the same three values the entry-0 member of this handler
 * run tests at 0x801F3E50. */
extern u16 D_80145AA4; /* @source 0x80145AA4 @kind unknown */
/* Second 16-entry byte table of this payload's tail blob, the row this handler
 * indexes with the low nibble of rand(); it holds only the bytes 0x1E, 0x3C
 * and 0x28. */
extern u8 D_801F5734[]; /* @source 0x801F5734 @kind table */

/* @source 0x801F3F78
 * @behavior Overlay step handler, entry 2 of the five-entry handler run based
 *           at 0x801F5804, which this target dispatches with the work record
 *           state byte at +1. When the shared area status byte D_80146865
 *           already reads 0xFE the body first resets the work-area header
 *           through func_80196070. It then publishes the third shared work base
 *           D_80146110 as the shared work pointer D_80146250 and as the
 *           scratchpad work cursor at 0x1F800044, keeping the caller cursor
 *           aside in a register. When the word at +0x34 of that base
 *           (D_80146144) reads above 0x33FFFF as a signed word the area ends
 *           exactly as the entry-1 and entry-3 members of the run do: the
 *           status byte takes the 0xFF end marker, the shared bytes D_80146866
 *           and D_80146867 are cleared, the shared sub-state byte D_80146875
 *           takes 0x14, the caller cursor is restored and the header of that
 *           restored record is reset through func_80196070 before returning.
 *           Otherwise it restores the caller cursor and splits on the status
 *           byte. While that byte still reads 0 it seeds work byte +9 with the
 *           byte the second 16-entry table at 0x801F5734 holds at the low
 *           nibble of rand(), then arms mode 3 by storing 3 into work byte +1
 *           through a fresh read of the cell rather than the held local. Once
 *           the status byte is set it advances the shared mode byte D_80146866
 *           when the shared pad word D_80145AA4 reads exactly 0x3000, 0x6000 or
 *           0x2000, the same three values the entry-0 member 0x801F3E50 tests:
 *           the incremented byte is stored unconditionally and the value 0x80
 *           is stored over it when the old byte already reads at least 0x10;
 *           only that saturated path continues into the work-area header reset
 *           through func_80196070, the same call site the end-area arm jumps to,
 *           while the unsaturated match and both non-matching paths return
 *           through the epilogue without calling. No jal in
 *           the payload reaches this address; its only in-image word is
 *           0x801F580C, entry 2 of that handler run. Takes no arguments and
 *           returns nothing; the 0x18-byte frame keeps the status-byte address
 *           in $s0 at 0x10 and $ra at 0x14 across the calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Name and shape evidence: the dispatch-table row at 0x801F5804 published by
 * this overlay's already-exact dispatcher
 * dispatchStateByte1Table5804World03Area13413_801F3E0C lists 0x801F3F78 as
 * entry 2, between the exact siblings
 * endAreaWhenThirdWorkWord34ExceedsLimitElseSeedRandomStatusByteAndArmMode2World03Area13413_801F3E8C
 * (entry 1) and
 * endAreaWhenThirdWorkWord34ExceedsLimitElseStepWorkByte9AndArmMode1World03Area13413_801F40C8
 * (entry 3), and the first 0x20 instructions of this body are identical to
 * theirs, so the status-pointer local in $s0, the volatile scratchpad cell, the
 * signed comparison against 0x33FFFF and the shared end-area path are that
 * family's behavior. The 16-entry table at 0x801F5734 sits directly after the
 * 0x801F5724 table the entry-1 member indexes and holds 0x1E/0x3C/0x28, so it
 * is a second random row of the same kind. The pad word test repeats the
 * entry-0 handler's three exact equality values, which is why the shared mode
 * byte D_80146866 advance sits in that arm. The mode-3 store into work byte +1
 * is this member's own arm, the counterpart of the entry-3 member's mode-1
 * store through the same cell.
 */
void endAreaWhenThirdWorkWord34ExceedsLimitElseSeedRandomWorkByte9AndArmMode3OrAdvanceSharedModeByteWorld03Area13413_801F3F78(
    void) {
  u8* work;
  u8* status;
  u8* mode;
  u8 state;
  u8 count;

  status = &D_80146865;
  if (*status == 0xFE) {
    func_80196070();
  }
  work = D_1F800044;
  D_80146250 = D_80146110;
  D_1F800044 = D_80146110;
  if (D_80146144 > 0x33FFFF) {
    *status = 0xFF;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146875 = 0x14;
    D_1F800044 = work;
  } else {
    state = *status;
    D_1F800044 = work;
    if (state == 0) {
      D_1F800044[9] = D_801F5734[rand() & 0xF];
      D_1F800044[1] = 3;
      return;
    }
    if (D_80145AA4 != 0x3000 && D_80145AA4 != 0x6000 && D_80145AA4 != 0x2000) {
      return;
    }
    mode = &D_80146866;
    count = *mode;
    *mode = count + 1;
    if (count < 0x10) {
      return;
    }
    *mode = 0x80;
  }
  func_80196070();
}
