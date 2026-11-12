#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The original re-materializes the cell and loads it again for every access
 * (ten lui/lw pairs for eleven dereferences in this body), because a byte store through
 * the loaded pointer may alias the cell itself, so the source reaches the cell
 * through this target-map symbol instead of a fixed-address lvalue. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Resource request wrapper of the shared main executable, outside this payload.
 * The original passes the literal 5 in the id slot and reads no result; the
 * exact sibling lifts of this family call it the same way and its address is
 * still named by the remaining asm of this target, so the generated
 * undefined_funcs_auto.txt binds it. */
extern void func_8014D6B8(u32 selector); /* @source 0x8014D6B8 @kind function */

/* Shared cue dispatcher of the main executable, outside this payload. The
 * original hands it work byte 0xA with its low bit cleared and 0x100 set, the
 * form the exact plchar sibling lifts of this same overlay family describe as
 * cue 0x100 plus the parity bit. */
extern void func_8015DF18(u16 cue); /* @source 0x8015DF18 @kind function */

/* @source 0x801EF5D0
 * @behavior Overlay header handler, index 6 of the handler-pointer run at
 *           0x801EEC04 (the payload word at 0x801EEC1C, its only in-image
 *           reference), and the run's entry 8 is the other lifted header handler
 *           of this payload while entry 7 is still an asm row. It seeds the
 *           scratchpad work
 *           object taken from the pointer cell 0x1F800044: byte 0x29 = 0, byte
 *           0x25 = 0x1D, byte 0x26 = 0, byte 0x24 = 0x84, byte 0x27 = 0xB4 and
 *           byte 0x28 = 0 in that order. One further cell load then feeds both
 *           byte 0x2B = 0 and halfword 0x2C = 0, and a fresh load feeds byte
 *           0x2A = 0, whose store the original keeps in the first call's delay
 *           slot. Every other store group re-reads the cell through its own
 *           load, so the source keeps one local per group and the named-cell
 *           form rather than a cached pointer. The body then requests resource
 *           5 through the resident wrapper func_8014D6B8, re-reads the cell and
 *           dispatches cue ((work byte 0xA & 1) | 0x100) through the shared cue
 *           dispatcher func_8015DF18, so the cue is 0x101 while byte 0xA reads
 *           odd and 0x100 while it reads even, and finally re-reads the cell and
 *           advances work byte 2 by one at byte width. It takes no arguments,
 *           returns nothing, and the 0x18-byte frame only keeps $ra at 0x10
 *           across the two calls. The roles of bytes 0x24, 0x25, 0x27 and 0x29,
 *           of the requested resource and of the selected cue are not
 *           established in this payload, so the name describes the writes and
 *           keeps the two literals with their offsets.
 *
 *           No byte-identical sibling exists: a search of all 396 payloads under
 *           out/binaries for this 0xD4-byte span finds no other copy, so the
 *           clone lever is declined. The statement order, the one-local-per-
 *           store-group form and the reload discipline come from the exact
 *           sibling of this family at magic003 0x801EFB8C, whose stored literals
 *           differ, and the seed's own credibility comes from the payload at
 *           magic052 0x801EEF84: that span's first 31 instructions equal this
 *           span's 9D0..A4C word for word, and the only further difference is the
 *           `addiu a0,$zero,5` this span hoists out of its first call, which
 *           proves the compiler takes one walk of the record for both bodies
 *           while that absolute word leaves no clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkHeaderKind84SelectorB4SelectCueByByteAParityRequestResourceAdvanceWorkByte2BmagicMagic13803_801EF5D0(
    void) {
  u8 *index;
  u8 *header;
  u8 *clear26;
  u8 *kind;
  u8 *selector;
  u8 *clear28;
  u8 *pair;
  u8 *clear2A;
  u8 *step;

  index = g_battle_work;
  index[0x29] = 0;
  header = g_battle_work;
  header[0x25] = 0x1D;
  clear26 = g_battle_work;
  clear26[0x26] = 0;
  kind = g_battle_work;
  kind[0x24] = 0x84;
  selector = g_battle_work;
  selector[0x27] = 0xB4;
  clear28 = g_battle_work;
  clear28[0x28] = 0;
  pair = g_battle_work;
  pair[0x2B] = 0;
  clear2A = g_battle_work;
  *(u16 *)(pair + 0x2C) = 0;
  clear2A[0x2A] = 0;
  func_8014D6B8(5);
  func_8015DF18((g_battle_work[0xA] & 1) | 0x100);
  step = g_battle_work;
  step[2]++;
}
