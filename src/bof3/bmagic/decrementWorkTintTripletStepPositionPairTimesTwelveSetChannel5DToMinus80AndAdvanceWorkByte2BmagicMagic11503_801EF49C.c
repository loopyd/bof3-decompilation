#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. The original folds the
 * symbolic address into each load (lui $vN,0x1F80 / lw $vN,68($vN)) and
 * re-materialises the cell before every store group, because a store through the
 * loaded pointer may alias the cell itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay image. The
 * generated undefined_syms_auto.txt already binds the address because the
 * remaining assembly spans func_801EF130, func_801EF27C, func_801EF5C4 and
 * func_801EF888 load it, so the name needs no map row. Words 0x34 and 0x38 read
 * here are the position pair and the cell is shared state, so the pointer stays
 * address-anchored at record width. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad word at 0x1F800000 that carries the 12-bit phase this body
 * publishes and then re-reads for the second sampler. The original builds the
 * address once into $s0 (lui 0x1F80 / addiu 0) because the pointer has to
 * survive the func_801782FC call, stores it with sw and re-reads it with lw, so
 * the body keeps one pointer local instead of naming the cell at each use. The
 * generated undefined_syms_auto.txt already binds the address. */
extern s32 D_1F800000; /* @source 0x1F800000 @kind data */

/* Shared no-argument main-executable update helper. The call site sets up no
 * argument register and this body overwrites the answer immediately, so the
 * return type moves no instruction here; the declared width follows the return
 * type other targets record for the same address. */
extern u8 func_8014D978(void);

/* Overlay-local angular sampler pair. Both take a 12-bit angle and return one
 * axis sample in the fixed-point scale this body multiplies by 0xC000 and shifts
 * right twelve; the repository documents the pair only as shared angular
 * helpers, so the declarations keep their address-anchored spelling. */
extern s32 func_801782FC(s32 angle);
extern s32 func_801783C8(s32 angle);

/* @source 0x801EF49C
 * @behavior Per-frame work-handler step of this overlay, entry index 2 of the
 *           eight-word code pointer run based at 0x801EF8C4 that the state-byte-2
 *           dispatcher dispatchStateByte2Tablef8c4BmagicMagic11503_801EF098
 *           indexes with the work object byte 2 scaled by four; no jal word of
 *           the payload targets this address, so this entry is selected through
 *           that table from outside the payload. It reads the scratchpad work
 *           object through the pointer cell at 0x1F800044, the first load issued
 *           before the 0x18-byte frame is created, and steps the object unsigned
 *           tint bytes 0x5D, 0x5E and 0x5F down by four in place (lbu / addiu -4
 *           / sb), re-materialising the cell before each store because a store
 *           through the loaded pointer may alias it. It then runs the shared
 *           no-argument update helper func_8014D978, whose returned byte it
 *           discards. It advances work byte 9 by one and publishes
 *           (byte 9 & 0x3F) << 6, the phase the angular pair consumes, into the
 *           scratchpad word 0x1F800000. It then steps the work position pair:
 *           word 0x34 takes the shared record word 0x34 read through the
 *           0x801EB4E0 cell plus twelve times the func_801782FC sample of that
 *           phase, and word 0x38 takes the record word 0x38 plus twelve times
 *           the func_801783C8 sample of the same phase re-read from 0x1F800000,
 *           each written as the 12-bit fixed-point product (sll 1 / addu / sll 14
 *           / sra 12). Between them it reads the record word 0x38 into its own
 *           local and writes -0x80 into tint byte 0x5D through the signed byte
 *           view, because the original materialises the immediate as addiu
 *           $v1,$zero,-0x80 rather than the 128 an unsigned store would fold to;
 *           that record load stays ahead of the tint store because a byte store
 *           and a record load may alias. After the tint store it re-reads the
 *           cell once more, and the pointer that load yields advances work byte
 *           2 after the word 0x38 store, which moves the dispatcher byte-2
 *           handler sequence forward. The 0x18-byte frame keeps $ra at
 *           0x14 and $s0 at 0x10, $s0 holding the 0x1F800000 address across the
 *           func_801782FC call. Takes no arguments, returns nothing through the
 *           jr $ra at 0x801EF5BC and reads or writes no other state; the roles of
 *           work bytes 9 and 2 and of the position words stay offset-anchored.
 *           The 296-byte span holds no byte-identical twin anywhere under
 *           out/binaries: a scan of all 396 extracted payloads for these bytes
 *           with every jal immediate word masked, decoded in the payload own
 *           little-endian byte order, returned only this target own copy at
 *           offset 0x89C, so the statement order above comes from the shape
 *           evidence of the exact siblings decrementWorkTintTripletByFourThen
 *           InvokeSharedHelperSetChannel5DToMinus80AndAdvanceWorkByte2 of this
 *           target (tint triplet decrement and signed channel clear) and
 *           decrementWorkByte9ThenSeedScratchAngleFromWorkByteBStepPositionPair
 *           BySampledAngularPairTimesTwelveClearByte9SetByteATo4AndAdvanceWork
 *           Byte2 of magic039 (the sampled position pair times twelve), whose
 *           records were measured exact, and not from a clone seed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkTintTripletStepPositionPairTimesTwelveSetChannel5DToMinus80AndAdvanceWorkByte2BmagicMagic11503_801EF49C(
    void) {
  u8 *record;
  u8 *record2;
  u8 *work;
  u8 *work2;
  u8 *work3;
  s32 *cell;
  s32 angle;
  s32 sample34;
  s32 sample38;
  s32 word38;
  u8 phase;

  g_battle_work[0x5D] -= 4;
  g_battle_work[0x5E] -= 4;
  g_battle_work[0x5F] -= 4;
  func_8014D978();
  phase = ++g_battle_work[9];
  cell = &D_1F800000;
  angle = (phase & 0x3F) << 6;
  *cell = angle;
  sample34 = func_801782FC(angle);
  record = D_801EB4E0;
  work = g_battle_work;
  *(s32 *)(work + 0x34) =
      *(s32 *)(record + 0x34) + ((sample34 * 0xC000) >> 12);
  sample38 = func_801783C8(*cell);
  record2 = D_801EB4E0;
  work2 = g_battle_work;
  word38 = *(s32 *)(record2 + 0x38);
  ((s8 *)work2)[0x5D] = -0x80;
  work3 = g_battle_work;
  *(s32 *)(work2 + 0x38) = word38 + ((sample38 * 0xC000) >> 12);
  work3[2]++;
}
