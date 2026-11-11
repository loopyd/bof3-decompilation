#include "bof3/bof3.h"

/* Main-RAM flag bank of the shared executable, outside this payload. The
 * world02/area111/13 handler at 0x801F3EE8, lifted exact, sets bit 0x28 of the
 * same bank through the same helper, the world04/area171/13 handler at
 * 0x801F3F24 sets the neighbouring overlay state byte in front of the same
 * request helper, and this target's generated undefined-syms map binds the
 * address already. The payload takes the address of the bank, not a word inside
 * it, so the declaration is the bank base.
 * @source 0x80144F28 @kind data
 */
extern u8 D_80144F28;

/* Shared block bit-set helper of the main executable (bank, bit index), outside
 * this payload and reached from this body only. The exact lifts that call it
 * treat it as a set-bit helper, so the parameters stay structural. This
 * target's generated undefined-funcs map binds the address.
 * @source 0x8015B580
 */
extern void func_8015B580(void* arg0, u8 bit_index);

/* Shared two-value request helper of the main executable, outside this payload
 * and reached from this body only. Its role is unresolved, so the parameters
 * stay structural; the world02/area100/13 neighbour at 0x801F3724 and the
 * world04/area171/13 handler at 0x801F3F24 both call it with a literal pair.
 * This target's generated undefined-funcs map binds the address.
 * @source 0x8015507C
 */
extern void func_8015507C(s32 arg0, s32 arg1);

/* Shared sound-cue dispatcher of the main executable, outside this payload. The
 * world04/area171/13 handler at 0x801F3F24 lifts it exact under the same role
 * and passes the same cue 0x109. This target's generated undefined-funcs map
 * binds the address.
 * @source 0x8015DF18
 */
extern void func_8015DF18(u32 cue);

/* Shared world-marker queue of the main executable (x, y, size), outside this
 * payload. include/bof3/world/area02713_internal.h records the role from the
 * sibling world targets, which also carry the (s16, s16, u32) prototype. This
 * target's generated undefined-funcs map binds the address.
 * @source 0x801A4BC0
 */
extern void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F3B20
 * @behavior Overlay handler with no caller in this payload: no jal and no table
 *           word of this overlay names it, so the overlay loader enters it. It
 *           sets bit 0x45 of the main-RAM flag bank D_80144F28 through the
 *           shared set-bit helper func_8015B580, passes the pair 0x4A and 0x1E
 *           to the shared request helper func_8015507C, queues the fixed sound
 *           cue 0x109 through the shared cue dispatcher func_8015DF18, and then
 *           queues four world markers of size 0xA1 through func_801A4BC0: the
 *           two-cell column at x = 0x49 with y = 0x1E and 0x1F, and the
 *           two-cell column at x = 0x4D with y = 0x33 and 0x34. The 0x18-byte
 *           frame holds only $ra at 0x10 across the seven calls, and the
 *           delayed `addiu` that loads each couple's later argument fills the
 *           following jal slot. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Evidence: the payload span 0x801F3B20-0x801F3B9B is one complete function of
 * 0x7C bytes with its own `addiu $sp, -0x18` prologue and a single `jr $ra`
 * epilogue, so it is neither data nor a fragment, and it fills its Splat row
 * exactly. The seven jal words and their literal delay slots were read from the
 * original instruction words: 0x801F3B30 func_8015B580 with a0 = &0x80144F28
 * and a1 = 0x45, 0x801F3B3C func_8015507C with a0 = 0x4A and a1 = 0x1E,
 * 0x801F3B44 func_8015DF18 with a0 = 0x109, and 0x801F3B54, 0x801F3B64,
 * 0x801F3B74 and 0x801F3B84 func_801A4BC0 with (0x49, 0x1E, 0xA1),
 * (0x49, 0x1F, 0xA1), (0x4D, 0x33, 0xA1) and (0x4D, 0x34, 0xA1).
 *
 * Naming evidence: the shared bit-set, request and cue helpers are named from
 * their exact lifts in other targets, so the name states what this body does to
 * them rather than what they are. The marker clauses follow the block grammar
 * of the exact sibling queueSharedUiElementBlock1CTo1Dx3SizeC0And20x15To16SizeA1
 * World04Area18813_801F4A54, whose four func_801A4BC0 calls carry the same
 * 0xA1-size family of literals, and the world02/area111/13 handler at
 * 0x801F3EE8 calls the same four-call marker site. Only the shared request
 * helper's role stays unresolved, and it is spelled as the neighbours do.
 *
 * Lever evidence: a byte scan of all 396 payload binaries for these 0x7C bytes
 * finds only this span at payload offset 0xF20 of emi/world02/area100/13.bin,
 * so the clone lever does not apply, and the discriminating immediates 0x45,
 * 0x4A/0x1E and the four marker triples appear nowhere else.
 */
void setSharedFlagBankBit45AndRequestPair4a1eThenQueueCue109AndMarkerBlocks49x1eTo1fAnd4dx33To34SizeA1World02Area10013_801F3B20(
    void) {
  func_8015B580(&D_80144F28, 0x45);
  func_8015507C(0x4a, 0x1e);
  func_8015DF18(0x109);
  func_801A4BC0(0x49, 0x1e, 0xa1);
  func_801A4BC0(0x49, 0x1f, 0xa1);
  func_801A4BC0(0x4d, 0x33, 0xa1);
  func_801A4BC0(0x4d, 0x34, 0xa1);
}
