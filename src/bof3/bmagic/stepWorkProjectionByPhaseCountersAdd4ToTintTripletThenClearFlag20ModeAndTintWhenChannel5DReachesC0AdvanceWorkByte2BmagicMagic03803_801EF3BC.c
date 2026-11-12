#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Every access in this body goes through a
 * fresh constant-address offset load of the cell, the form this target's other
 * work-object handlers use, so the local declaration keeps the named cell and
 * each statement reads it again. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address, so
 * the name needs no map row, and every access here is word-addressed at a fixed
 * offset, so the local declaration keeps the byte view the record starts from. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad cell that publishes the rotation phase handed to the shared
 * samplers, and the scratchpad cell that holds the third accumulator's scale.
 * Both are plain fixed addresses the compiler reaches as an offset load, so the
 * generated undefined_syms_auto.txt row is the binding of the first and the
 * second needs none: a D_<ADDRESS> name binds without a map row. */
extern s32 D_1F800000; /* @source 0x1F800000 @kind data */
extern s32 D_1F800004; /* @source 0x1F800004 @kind data */

/* Shared per-frame update helper of the main executable, outside this overlay's
 * image; every call site in this overlay sets no argument and discards $v0, so
 * it is declared void with no arguments. */
extern void func_8014D978(void); /* @source 0x8014D978 @kind function */

/* Shared phase sampler of the main executable, outside this overlay's image: it
 * takes the angle published in scratchpad word 0x1F800000 and answers a signed
 * sample the bmagic draw and step bodies scale down by twelve bits. The paired
 * sampler func_801783C8 reads the same angle and answers the quadrature
 * sample. */
extern s32 func_801782FC(s32 angle); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 @kind function */

/* @source 0x801EF3BC
 * @behavior Step handler of this overlay's work-state run. It first runs the
 *           shared per-frame update func_8014D978, discarding the answer, then
 *           adds 4 to the work object's three tint channels at bytes 0x5D, 0x5E
 *           and 0x5F with a byte-width read-modify-write (lbu / addiu 4 / sb),
 *           re-reading the pointer cell for each of the three.
 *           It advances the object's phase word 0x0C by one, publishes
 *           ((word 0x0C) & 0x7F) << 5 as the rotation angle in scratchpad word
 *           0x1F800000, and samples func_801782FC at that angle; the resulting
 *           signed sample times 0x20000, arithmetically shifted down twelve
 *           bits, offsets shared record word 0x34 and is stored in work word
 *           0x34. It then re-reads the published angle and samples
 *           func_801783C8 at it, and the same scaled sample offsets shared
 *           record word 0x38 into work word 0x38. Next it advances the object's
 *           second phase word 0x14 by one, publishes ((word 0x14) & 0x3F) << 6
 *           as the angle, stores 0x800 in scratchpad word 0x1F800004, and
 *           samples func_801782FC at that angle; that sample times the stored
 *           0x800, with no shift, offsets shared record word 0x3C into work word
 *           0x3C.
 *           Finally it tests the object's tint channel 0x5D as a signed byte:
 *           only when that channel reads -0x40 (0xC0) does it clear bit 0x20 of
 *           the object's flag byte 0 through the 0xDF mask, zero mode byte 0x5C
 *           and the three tint channels at 0x5D, 0x5E and 0x5F, and increment
 *           the object's state byte 2 to advance the run. Each of those stores
 *           re-reads the pointer cell, and the flag clear reuses the cell read
 *           that the last accumulator store already made.
 *           Takes no arguments and returns nothing; the 0x18-byte frame only
 *           keeps $ra at 0x14 and the scratchpad cell address at 0x10 across the
 *           four calls, and no word in this payload holds 0x801EF3BC, so the
 *           only incoming path is outside the overlay. Roles of the phase words
 *           0x0C and 0x14, the tint channels, flag byte 0 and mode byte 0x5C are
 *           not established here, so they stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkProjectionByPhaseCountersAdd4ToTintTripletThenClearFlag20ModeAndTintWhenChannel5DReachesC0AdvanceWorkByte2BmagicMagic03803_801EF3BC(void) {
  s32 *angle;
  s32 sample;

  func_8014D978();
  D_1F800044[0x5D] += 4;
  D_1F800044[0x5E] += 4;
  D_1F800044[0x5F] += 4;

  *(s32 *)(D_1F800044 + 0x0C) += 1;
  angle = &D_1F800000;
  *angle = (*(u32 *)(D_1F800044 + 0x0C) & 0x7F) << 5;
  sample = func_801782FC(*angle);
  *(s32 *)(D_1F800044 + 0x34) = *(s32 *)(D_801EB4E0 + 0x34) + ((sample * 0x20000) >> 12);

  sample = func_801783C8(*angle);
  *(s32 *)(D_1F800044 + 0x38) = *(s32 *)(D_801EB4E0 + 0x38) + ((sample * 0x20000) >> 12);

  *(s32 *)(D_1F800044 + 0x14) += 1;
  *angle = (*(u32 *)(D_1F800044 + 0x14) & 0x3F) << 6;
  D_1F800004 = 0x800;
  sample = func_801782FC(*angle);
  *(s32 *)(D_1F800044 + 0x3C) = *(s32 *)(D_801EB4E0 + 0x3C) + sample * D_1F800004;

  if (((s8 *)D_1F800044)[0x5D] == -0x40) {
    D_1F800044[0] &= 0xDF;
    D_1F800044[0x5C] = 0;
    D_1F800044[0x5D] = 0;
    D_1F800044[0x5E] = 0;
    D_1F800044[0x5F] = 0;
    D_1F800044[2]++;
  }
}
