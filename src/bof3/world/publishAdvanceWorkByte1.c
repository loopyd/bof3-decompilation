#include "bof3/bof3.h"

/* Shared saved work triplet: the three consecutive 32-bit words the overlay
 * state handlers of this target seed their own records from. Declared plain,
 * because the original loads each word immediately before the store that
 * consumes it. */
extern s32 D_80146954;
extern s32 D_80146958;
extern s32 D_8014695C;

/* Work record pointer published at the scratchpad cell 0x1F800044. Declared
 * plain, not volatile: the original re-materialises the cell address and
 * reloads the pointer for the group after the call, and the plain cell
 * already forces that. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F44A0 @kind data
 * Base pointer of the work record this overlay publishes for itself. Entry 0
 * of the handler run at 0x801F3DAC stores the literal 0x800E4800 into this
 * cell, and the later entries of the same run read it back as the base to
 * seed and to render. The cell sits in the payload tail, so its image value
 * is zero until this initializer runs. */
extern u8* D_801F44A0;

void stateClearCursorByte(void);

/* @source 0x801F2C48
 * @behavior Area initializer, entry 0 of the six-word handler run based at
 *           0x801F3DAC that this target dispatcher
 *           dispatchByte1Table3dac indexes with
 *           the state byte at +1 of the work record published at the
 *           scratchpad pointer cell 0x1F800044. Copies the shared saved
 *           triplet D_80146954, D_80146958 and D_8014695C into the words at
 *           +0x34, +0x38 and +0x3C of that scratch work record, then primes
 *           the fixed work area at 0x800E4800 from those same three words:
 *           the pair at +0x00/+0x04 and +0x10/+0x14 receives the first two
 *           words unmodified, the height words at +0x08 and +0x18 receive the
 *           third word biased by 0x08000000, and the words at +0x0C and +0x1C
 *           to +0x1F are left untouched. It publishes 0x800E4800 as the work
 *           record base D_801F44A0, writes 0x20 at +0x22, zero at +0x23 and a
 *           zero halfword at +0x20,
 *           rewinds the 0x800E486C record cursor through 0x801F3834, and then
 *           arms the countdown byte at +0x09 of the scratch work record with
 *           0x10 and advances its state byte at +0x01 so the next frame
 *           dispatches entry 1 at 0x801F2D34. Takes no arguments and returns
 *           nothing; its 0x18-byte frame holds $ra at 0x10 across the call.
 *
 * Name evidence: the only in-image word naming 0x801F2C48 is entry 0 of the
 * pointer run at 0x801F3DAC, the table the exact dispatcher at 0x801F2C04
 * indexes with the work record state byte at +0x01. The exact sibling
 * stateClearCursorByte records
 * this span as the area initializer that reaches it after priming the shared
 * work words at 0x800E4800 through 0x800E4823, and the exact entry 4 handler
 * seedPublishedRecordWords
 * reads the D_801F44A0 base this body publishes. The seed, publish, countdown
 * and state-byte roles come from this body's own stores and offsets.
 *
 * Shape evidence: statement order, operand forms and the reload points follow
 * the exact sibling seedWorkWords34To3CAnd0CTo14DispatchCue207AndAdvanceWorkByte1World03Area13213_801F30C0,
 * which seeds a record from the same saved triplet at the same +0x34..+0x3C
 * offsets and then advances the same state byte, and the countdown arm and
 * state-byte increment follow the exact same-target entry 4 handler named
 * above. This function's own offsets, literals and the two-cell scratch view
 * come from its original bytes. The clone lever is declined on one payload
 * proof: the 0x74-byte leading run of these 0xEC bytes, which holds no
 * payload-local word, occurs only at payload offset 0x48 of the 6316-byte
 * payload of this target, so no byte-identical twin of the range exists
 * anywhere under out/binaries.
 *
 * Matching note: the base pointer into the fixed work area is a value, so the
 * compiler materialises it in a register once and the zero-offset store reuses
 * that register as its base while the other offsets keep their absolute
 * address form. The first measured spelling put that publish assignment ahead
 * of the whole store group and scored 53/59 with 236->240 bytes, because the
 * constant then materialised in the wrong register and the store that reuses
 * it pulled an extra pointer load and a load-delay nop. Moving the assignment
 * behind the word stores scored 57/59 with 236->236 bytes, leaving the publish
 * store scheduled one store group too early. The final spelling leaves the
 * assignment after the +0x14 store, which places the publish store between the
 * load and the store of that word and measured 59/59 at 236->236 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishAdvanceWorkByte1(
    void) {
  u8* record;
  u8* work;
  u8* state;

  record = D_1F800044;
  *(s32*)(record + 0x34) = D_80146954;
  *(s32*)(record + 0x38) = D_80146958;
  *(s32*)(record + 0x3C) = D_8014695C;
  PSX_REF(s32, 0x800E4800u) = *(s32*)(record + 0x34);
  PSX_REF(s32, 0x800E4804u) = *(s32*)(record + 0x38);
  PSX_REF(s32, 0x800E4808u) = *(s32*)(record + 0x3C) + 0x08000000;
  PSX_REF(s32, 0x800E4810u) = *(s32*)(record + 0x34);
  PSX_REF(s32, 0x800E4814u) = *(s32*)(record + 0x38);
  D_801F44A0 = PSX_PTR(u8, 0x800E4800u);
  PSX_REF(s32, 0x800E4818u) = *(s32*)(record + 0x3C);
  PSX_REF(u8, 0x800E4822u) = 0x20;
  PSX_REF(u8, 0x800E4823u) = 0;
  PSX_REF(u16, 0x800E4820u) = 0;
  stateClearCursorByte();
  work = D_1F800044;
  work[9] = 0x10;
  state = D_1F800044;
  state[1]++;
}
