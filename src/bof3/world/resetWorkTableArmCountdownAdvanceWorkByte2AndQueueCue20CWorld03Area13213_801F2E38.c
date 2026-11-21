#include "bof3/bof3.h"

void clearWorkTableClaimBytesWorld03Area13213_801F385C(void);

/* Cue submission of the main executable, outside this overlay's payload: takes
 * the cue id in $a0 and returns nothing. This target's generated
 * undefined_funcs_auto.txt binds the address. */
void func_8015DF18(u16 cue);

/* Shared published work-pointer cell: the original re-materialises the cell
 * address and reloads the pointer for each of the two access groups below. The
 * reload between them is forced by the published store at +0x09, which may
 * alias the cell itself, so the cell needs no volatile annotation. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F2E38
 * @behavior Entry 0 of the byte-2 handler pointer run based at 0x801F3DEC,
 *           whose first three words the payload holds as 0x801F2E38, 0x801F2E88
 *           and 0x801F2F20. The run is indexed with the work record's unsigned
 *           byte at +0x02 by this target's exact dispatcher
 *           dispatchStateByte2Table3decWorldWorld03Area13213_801F2DF4, so this
 *           body is the first step of that sequence. It clears every claim byte
 *           of the 0x20-record work table through the exact helper at
 *           0x801F385C, then arms the published record's countdown byte at
 *           +0x09 with 0x40, reloads the cell and increments the record state
 *           byte at +0x02, which hands the record to entry 1 at 0x801F2E88, and
 *           queues cue 0x20C through 0x8015DF18. It takes no arguments, returns
 *           nothing and its 0x18-byte frame holds only $ra at 0x10 across the
 *           two calls.
 *
 * Dispatch evidence: the payload words 0x801F2E38, 0x801F2E88, 0x801F2F20,
 * 0x801F2F98, 0x801F3048 and 0x801F30C0 fill the run based at 0x801F3DEC up to
 * the next declared handler run 0x801F3DF8, and the exact dispatcher at
 * 0x801F2DF4 scales the record byte at +0x02 by four onto that base and jalrs
 * the selected entry, so this span is reached indirectly and its argument roles
 * stay structural.
 *
 * Naming evidence: entry 1 of the same run, the exact
 * claimSeedWorkRecordEveryFourthFrameRunRecordPassThenAdvanceWorkByte2WhenCountdownExpiresWorld03Area13213_801F2E88,
 * consumes the two fields this body writes: it finds a free record straight
 * after the table reset, counts the byte at +0x09 down and advances the byte at
 * +0x02 to reach the next step, so +0x09 is the countdown, +0x02 the dispatch
 * byte and the callee the table reset. The cue literal 0x20C is this span's own
 * argument word.
 *
 * Representation: the cell is spelled by this target's `D_1F800044` name and
 * read once per access group, so the original's two lui/lw pairs stay; the
 * countdown store is the literal 0x40 and the state increment is a byte
 * read-modify-write on the second load; the cue argument is declared u16 as in
 * the exact lifts that queue cue ids through 0x8015DF18.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetWorkTableArmCountdownAdvanceWorkByte2AndQueueCue20CWorld03Area13213_801F2E38(
    void) {
  u8* work;
  u8* state;

  clearWorkTableClaimBytesWorld03Area13213_801F385C();
  work = D_1F800044;
  work[9] = 0x40;
  state = D_1F800044;
  state[2]++;
  func_8015DF18(0x20C);
}
