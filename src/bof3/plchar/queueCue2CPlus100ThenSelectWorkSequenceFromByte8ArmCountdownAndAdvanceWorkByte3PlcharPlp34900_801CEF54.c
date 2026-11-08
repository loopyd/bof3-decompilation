#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original loads the cell once before frame creation, re-reads it after the cue
 * call and again before each of the two stores, because a store through the
 * pointer may alias the cell itself; the source keeps the named-symbol form and
 * evaluates it at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Cue queue routine of the shared EXE image, outside this payload. The original
 * calls it through a plain `jal` with the work halfword 0x2C plus 0x100 in $a0,
 * computes that argument before the frame is created, and the added constant
 * issues in the call's own delay slot. Splat binds its address through this
 * target's generated undefined_funcs_auto.txt, and the remaining asm rows of
 * this payload reference it too. */
void func_8015DF18(u32 arg0);

/* Work-sequence selector of the shared EXE image, outside this payload. The
 * original passes the sequence ((work byte 8 - 1) / 2 + 0x42) truncated to a
 * byte, which keeps both divisions signed, and masks the value in the call's
 * own delay slot. Splat binds its address through this target's generated
 * undefined_funcs_auto.txt, and other asm rows of this payload reference it. */
u32 func_8014D8D4(u8 arg0);

/* Seeded from the lift of the same body at plp034 0x801CFC7C. The clone dry-run
 * compared this 120-byte span byte-identical to that representative range in the
 * payload, so the statement order, the pointer-cell access form and the
 * declaration style were carried over with this function's own name and this
 * payload's own dispatch facts.
 */
/* @source 0x801CEF54
 * @behavior Overlay step dispatched on work-record byte 3: the task tables of
 *           this payload place its address at the pointer word 0x801CF614, which
 *           is entry 10 of the sixteen-word code-pointer run based at 0x801CF5EC,
 *           so dispatchWorkByte3Tablef614PlcharPlp34900_801CEF10 reaches it when
 *           work byte 3 reads zero. It reads the work record published at the
 *           scratchpad pointer cell 0x1F800044 and, in order, queues the cue
 *           taken from the record's unsigned halfword 0x2C plus 0x100 through
 *           func_8015DF18, selects the shared work sequence
 *           ((record byte 8 - 1) / 2 + 0x42) truncated to a byte through
 *           func_8014D8D4, arms the countdown byte 0x0A with 8 and sets the
 *           dispatch byte 3 to 1. Both calls answer nothing it keeps; it has no
 *           `jal` caller and takes no arguments. The subtraction leaves the
 *           division signed, so a record byte 8 of zero halves -1 to zero and
 *           still selects sequence 0x42. The pointer cell is loaded before the
 *           frame is created and re-read after the cue call and before each of
 *           the two stores; the 0x18-byte frame keeps only $ra at 0x10, the
 *           countdown constant is materialised after the second call and the
 *           dispatch constant 1 is materialised before $ra is restored. The
 *           120-byte span at payload offset 0xB54 sits exactly between the two
 *           declared rows that bound it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueCue2CPlus100ThenSelectWorkSequenceFromByte8ArmCountdownAndAdvanceWorkByte3PlcharPlp34900_801CEF54(
    void) {
  func_8015DF18(*(u16*)(D_1F800044 + 0x2C) + 0x100);
  func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
  D_1F800044[0xA] = 8;
  D_1F800044[3] = 1;
}
