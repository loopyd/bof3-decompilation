#include "bof3/bof3.h"

/* Shared main-executable update pass. The exact sibling
 * dispatchWorkByte2ThenCallUpdatePassesBmagicMagic05003_801EEF4C calls the same
 * helper for a frame whose work record is in an active state. */
void func_800AF764(void);

/* The two draw variants this entry selects by the work object's byte 0x0B. Both
 * are still assembly in this overlay; each one emits SetPolyFT4, GetTPage,
 * GetClut and func_8014E5A0 prims over the work halfwords at 0x2E and 0x30. */
void func_801EED94(void);
void func_801EEF5C(void);

void func_800A9820(u8 arg0);
void func_801E5988(void);

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map.
 * The original reads the cell once before the byte 0x0B dispatch and once more
 * for the byte 0x09 countdown, one lui/lw pair per access group, so the cell is
 * bound by name rather than through a constant-address accessor. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * func_800A9820 consumes. It lives outside this overlay, so it keeps its
 * address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword at 0x801462E8. The siblings of this family raise
 * its bit 0x4 the same way; it lives outside this overlay, so it keeps its
 * address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EECE4
 * @behavior Work byte 1 state 1 of the MAGIC080 overlay: entry 1 of the
 *   two-entry handler table func_801EEC04 builds in its own frame and
 *   dispatches on the scratchpad work object state byte 1, which state 0 at
 *   0x801EEC5C advances into. It first calls the shared update pass
 *   func_800AF764, re-reads the work-object pointer cell 0x1F800044 and
 *   subtracts 0x24 from the unsigned work halfword 0x2E at halfword width, that
 *   store filling the branch delay slot. It then calls the draw variant
 *   func_801EEF5C when the work object byte 0x0B (the random mask byte state 0
 *   seeds) is non-zero and func_801EED94 when it is zero. After that group it
 *   re-reads the pointer cell and decrements work byte 0x09 with a byte-width
 *   read-modify-write whose stored value truncated to eight bits supplies the
 *   test, so the countdown byte reaching zero loads the shared battle byte at
 *   0x80146384 unsigned and passes it to func_800A9820, raises bit 0x4 of the
 *   shared battle halfword at 0x801462E8 and invokes func_801E5988. It takes no
 *   argument, returns nothing, and the 0x18-byte frame only keeps $ra across the
 *   calls. Statement order, operand forms and declaration style follow the exact
 *   siblings of this family, in particular
 *   forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZero
 *   BmagicMagic10603_801EEE80 for the guarded flag-4 group and
 *   incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAnd
 *   InvokeHelperWhenZeroBmagicMagic14503_801EF0A0 for the truncated countdown
 *   test, and every literal (0x24, 0x2E, 0x0B, 9) is this body's own, read from
 *   the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decayWorkHalfword2EThenDrawByWorkByteBAndRaiseFlag4WhenByte9ExpiresBmagicMagic08000_801EECE4(
    void) {
  u8 *work;

  func_800AF764();
  work = g_battle_work;
  *(u16 *)(work + 0x2E) -= 0x24;
  if (work[0xB] != 0) {
    func_801EEF5C();
  } else {
    func_801EED94();
  }
  if (--g_battle_work[9] == 0) {
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
